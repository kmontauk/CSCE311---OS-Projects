#include <iostream>
#include <csignal>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <thread> // Only for sleeping while creating and debugging.
#include <chrono>
#include "./client.h"

#define SHM_SIZE 0x400

struct shmbuf* shmp;

// Global semaphore variable
sem_t* client_semaphore;
sem_t* server_semaphore;

// Global shared memory name
const char* shm_name = "/my_shared_memory";

// Signal handler to destroy the semaphore
void destroySemaphore(int signal) {
    sem_unlink("/my_semaphore");
    sem_unlink("/my_semaphore2");
    sem_close(server_semaphore);
    sem_close(client_semaphore);
    exit(0);
}

int main() {
    // Set up signal handler to destroy the semaphore
    signal(SIGINT, destroySemaphore);
    signal(SIGTERM, destroySemaphore);

    // Attempt to destroy the semaphores before creating them
    sem_unlink("/my_semaphore");
    sem_unlink("/my_semaphore2");

    // Create the semaphores
    client_semaphore = sem_open("/client_semaphore", O_CREAT | O_EXCL, 0644, 0);
    server_semaphore = sem_open("/server_semaphore", O_CREAT | O_EXCL, 0644, 0);
    if (client_semaphore == SEM_FAILED || server_semaphore == SEM_FAILED) {
        if (errno != EEXIST) {
            std::cerr << "Failed to create semaphore" << std::endl;
            destroySemaphore(1);
        }
        std::cerr << "Failed to create semaphore" << std::endl;
        destroySemaphore(1);
    }


    // Server main loop
    while (true) {
        // Signal the client that the server is ready
        sem_post(server_semaphore);
        std::cout << "Server is ready" << std::endl;

        // Lock the barrier semaphore
        sem_wait(client_semaphore);
        std::cout << "Server is processing client request" << std::endl;

        // Connect to shared memory initialized by the client
        int shm_fd = shm_open(shm_name, O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open");
            destroySemaphore(1);
        }

        // Set the size of the shared memory object
        if (ftruncate(shm_fd, SHM_SIZE) == -1) {
            perror("ftruncate");
            destroySemaphore(1);
        }

        // Map the shared memory object into the process's address space
        //char* shmp = (char*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        shmp = (shmbuf*)mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shmp == MAP_FAILED) {
            perror("mmap");
            destroySemaphore(1);
        }

        // Read the file name and path from the shared memory
        std::string fileName;
        for (int i = 0; i < SHM_SIZE; i++) {
            if (shmp->buf[i] == '!') {
                break;
            }
            fileName += shmp->buf[i];
        }

        // Format for the lines_str is "!<lines_count>"
        std::string lines_str = shmp->buf + fileName.length();
        // Extract the lines_count from the lines_str
        int delimiter_index = lines_str.find('!');
        lines_str = lines_str.substr(delimiter_index);
        int lines_count = std::stoi(lines_str.substr(1));

        // Print the path and lines_str
        std::cout << "File name: " << fileName << std::endl;
        std::cout << "Lines count: " << lines_count << std::endl;

        // Open the file
        FILE* file = fopen(fileName.c_str(), "r");
        if (file == NULL) {
            perror("fopen");
            destroySemaphore(1);
        }
        // Transfer the file to shared memory
        snprintf(shmp->buf, SHM_SIZE, "%s", file);


        

    }

    return 0;
}
