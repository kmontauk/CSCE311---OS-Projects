#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>

#define SHM_SIZE 1024

int main(int argc, char* argv[]) {
    // Generate a unique key for the shared memory segment
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    // Create the shared memory segment
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Attach to the shared memory segment
    char* sharedMemory = (char*)shmat(shmid, NULL, 0);
    if (sharedMemory == (char*)-1) {
        perror("shmat");
        return 1;
    }

    
    // Write the file name and path to the shared memory
    const char* fileName = argv[1];
    int lines_count = std::stoi(argv[2]);
    std::string lines_str = std::to_string(lines_count);
    lines_str += "!";


    strncpy(sharedMemory, fileName, SHM_SIZE);

    // Detach from the shared memory segment
    if (shmdt(sharedMemory) == -1) {
        perror("shmdt");
        return 1;
    }

    std::cout << "File name and path have been written to shared memory." << std::endl;

    // Read from the shared memory segment
    // Attach to the shared memory segment again
    sharedMemory = (char*)shmat(shmid, NULL, 0);
    if (sharedMemory == (char*)-1) {
        perror("shmat");
        return 1;
    }

    std::string data(sharedMemory);

    // Print the contents of the shared memory
    std::cout << "Data read from shared memory: " << data << std::endl;

    return 0;
}