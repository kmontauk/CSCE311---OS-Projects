#include <iostream>
#include <csignal>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <thread> // Only for sleeping while creating and debugging.
#include <chrono>
#include <fstream>
#include <vector>
#include <string>
#include "./client.h"
using std::cout, std::endl, std::string, std::vector;

#define SHM_SIZE 0x400

struct shmbuf* shmp;

// Global semaphore variable
sem_t* client_semaphore;
sem_t* server_semaphore;

// Global shared memory name
const char* shm_name = "/client_shared_memory";

// Signal handler to destroy the semaphore
void destroySemaphore(int signal) {
    sem_unlink("/client_semaphore");
    sem_unlink("/server_semaphore");
    sem_close(server_semaphore);
    sem_close(client_semaphore);
    exit(0);
}

int main() {
    // Set up signal handler to destroy the semaphore
    signal(SIGINT, destroySemaphore);
    signal(SIGTERM, destroySemaphore);

    // Attempt to destroy the semaphores before creating them
    sem_unlink("/client_semaphore");
    sem_unlink("/server_semaphore");

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
        std::string file_path;
        for (int i = 0; i < SHM_SIZE; i++) {
            if (shmp->buf[i] == '!') {
                break;
            }
            file_path += shmp->buf[i];
        }

        // Format for the lines_str is "!<lines_count>"
        std::string lines_str = shmp->buf + file_path.length();
        // Extract the lines_count from the lines_str
        int delimiter_index = lines_str.find('!');
        lines_str = lines_str.substr(delimiter_index);
        int lines_count = std::stoi(lines_str.substr(1));

        // Print the path and lines_str
        std::cout << "File name: " << file_path << std::endl;
        std::cout << "Lines count: " << lines_count << std::endl;

        // Open the file using fstream
        std::ifstream file(file_path);
        if (!file) {
            std::cerr << "Unable to open file: " << file_path << std::endl;
            destroySemaphore(1);
        }

        std::vector<std::string> lines(lines_count); 
        int i = 0;
        // Transfer the file to a local vector
        std::string line;
        while (i < lines_count && std::getline(file, line)) {
            lines[i] = line;
            lines[i] += '!'; // Add a delimiter to the end of the line
            cout << lines[i] << endl;
            ++i;
        }
        // Convert the vector to a string 
        std::string string_of_all_lines = "!"; // Start with a delimiter
        for (int i = 0; i < lines_count; i++) {
            string_of_all_lines += lines[i];
        }


        // Pass the string to shared memory.
        strncpy(shmp->buf, string_of_all_lines.c_str(), SHM_SIZE);
        
    }

    return 0;
}