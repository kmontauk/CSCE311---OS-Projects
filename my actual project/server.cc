#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int main() {
    // Create or get the shared memory segment
    key_t key = ftok("/tmp", 'A');
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        std::cerr << "Failed to create or get shared memory segment." << std::endl;
        return 1;
    }

    // Attach to the shared memory segment
    char* sharedData = (char*)shmat(shmid, nullptr, 0);
    if (sharedData == (char*)-1) {
        std::cerr << "Failed to attach to shared memory segment." << std::endl;
        return 1;
    }

    std::string data(sharedData);
    // Read data from the shared memory
    std::cout << "Data from client: " << data << std::endl;

    // Detach from the shared memory segment
    if (shmdt(sharedData) == -1) {
        std::cerr << "Failed to detach from shared memory segment." << std::endl;
        return 1;
    }

    // Remove the shared memory segment (optional)
    if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
        std::cerr << "Failed to remove shared memory segment." << std::endl;
        return 1;
    }

    return 0;
}