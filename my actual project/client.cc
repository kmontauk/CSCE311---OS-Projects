#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <semaphore.h>
#include <thread> // Only for sleeping while creating and debugging.
#include <chrono>
#include "./client.h"

#define SHM_SIZE 0x400

struct shmbuf* shmp;

int main(int argc, char* argv[]) {
    // Input checking
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <file path> <number of lines>" << std::endl;
        return 1;
    }

    // open existing semaphores located on the server
    sem_t* client_semaphore = sem_open("/client_semaphore", 0);
    sem_t* server_semaphore = sem_open("/server_semaphore", 0); 
    if (client_semaphore == SEM_FAILED || server_semaphore == SEM_FAILED) {
        perror("sem_open");
        // 
        while (true) {
            printf("open. up. the server. stop. having it. be closed. \n");
            std::this_thread::sleep_for(std::chrono::seconds(5));
            client_semaphore = sem_open("/client_semaphore", 0);
            server_semaphore = sem_open("/server_semaphore", 0); 
            if (client_semaphore != SEM_FAILED && server_semaphore != SEM_FAILED) {
                break;
            }
        }
    }

    // Generate a unique name for the shared memory object
    const char* shm_name = "/my_shared_memory";

    // Create the shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Map the shared memory object into the process's address space
    //char* shmp = (char*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shmp = (shmbuf*) mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shmp == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Wait for the server to signal that it is ready
    while(server_semaphore == 0) {
        //sem_t* server_semaphore = sem_open("/my_semaphore2", 0); 
    }


    // Create the file name, path, and lines_str
    const char* file_path = argv[1];
    //std::string lines_str = "!";
    int lines_count = std::stoi(argv[2]);
    std::string lines_str = std::to_string(lines_count);
    const char* lines_count_char = lines_str.c_str();
    //lines_str += std::to_string(lines_count);
    //std::cout << lines_str << std::endl;
    
    // Copy the file path and lines_str to the shared memory
    strncpy(shmp->buf[0], file_path, SHM_SIZE); // file path copy
    strncpy(shmp->buf[1], lines_count_char, SHM_SIZE); // lines_count copy

    //strncat(shmp->buf[1], lines_str.c_str(), SHM_SIZE - strlen(shmp->buf[1]) - 1); // lines_count copt

    // Unmap the shared memory object
    if (munmap(shmp, SHM_SIZE) == -1) {
        perror("munmap");
        return 1;
    }

    // Signal the server that the shared memory is ready
    sem_post(client_semaphore);

    std::cout << "File name, path, and lines_str have been written to shared memory." << std::endl;

    // Map the shared memory object into the process's address space again
    shmp = (shmbuf*) mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shmp == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    // sleep for 1 seconds
    std::this_thread::sleep_for(std::chrono::seconds(1));


    std::string data(shmp->buf[0]);
    std::string data2(shmp->buf[1]);

    // Print the contents of the shared memory
    std::cout << "Data read from shared memory: " << data << std::endl;
    std::cout << "Data read from shared memory: " << data2 << std::endl;
    
    for(int i = 0; i < lines_count; i++) {
        std::cout << shmp->buf[i] << std::endl;
    }

    // Unmap the shared memory object again
    if (munmap(shmp, SHM_SIZE) == -1) {
        perror("munmap");
        return 1;
    }

    // Close the shared memory object
    if (close(shm_fd) == -1) {
        perror("close");
        return 1;
    }

    // Remove the shared memory object
    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
        return 1;
    }

    return 0;
}
