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
#include <vector>
#include <fstream>
using std::cout;
using std::string;
using std::endl;

#define SHM_SIZE 0x400

struct shmbuf* shmp;

// Global semaphore variable
sem_t* client_semaphore;
sem_t* server_semaphore;

// Global shared memory name
const char* shm_name = "/my_shared_memory";

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

        // Lock the barrier semaphore (wait for client to be ready)
        sem_wait(client_semaphore);
        std::cout << "Server is processing client request" << std::endl;

        // Connect to shared memory initialized by the client
        int shm_fd = shm_open(shm_name, O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open");
            destroySemaphore(1);
        }

        // Map the shared memory object into the process's address space
        //char* shmp = (char*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        shmp = (shmbuf*) mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shmp == MAP_FAILED) {
            perror("mmap");
            destroySemaphore(1);
        }

        // Read the file name and path from the shared memory
        std::string file_path;
        file_path = shmp->buf[0];
        std::cout << "File: " << file_path << std::endl;

        string lines_str = shmp->buf[1]; 
        cout << "lines_str: " << lines_str << endl;
        int lines_count = std::stoi(lines_str);
        
        // Print the path and lines_str
        std::cout << "File: " << file_path << std::endl;
        std::cout << "Lines count: " << lines_count << std::endl;

        /** Open the file
        FILE* file = fopen(file_path.c_str(), "r");
        if (file == NULL) {
            perror("fopen");
            destroySemaphore(1);
        }

        int i = 0;
        
        // Transfer the file to a local buffer
        while (i < lines_count) {
            if (fgetc(file) == EOF) break;
            lines[i] = fgetc(file);
         }
        **/

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
        i = 0;
        for (string line : lines) {
            //cout << "here?" << endl;
            int j = 0;
            while (true) {
                cout << "wya?" << endl;
                shmp->buf[i][j] += line.c_str()[j];
                if (line.c_str()[j] == '!') {
                    break;
                }         
                j++;
            }
            i++; 
        }
    }
}