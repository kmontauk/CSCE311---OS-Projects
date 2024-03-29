#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>

#define SHM_SIZE 0x400

using std::string;
using std::cout;
using std::endl;

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

    // Format data to separate the file path and number of lines
    char temp = data[0];
    int i = 0;
    string file_path = "";
    while (temp != '!') {
        file_path += data[i];
        temp = data[i];
        i++;
    }
    string lines_str = data.substr(i, data.length() - i);
    file_path = file_path.substr(0, file_path.length() - 1);
    int lines_count = stoi(lines_str);

    // Print the lines_str and file_path
    std::cout << "File path: " << file_path << std::endl;
    std::cout << "Lines: " << lines_count << std::endl;
    
    // Open the file or signal to client that the file could not be opened
    FILE* file = fopen(file_path.c_str(), "r");
    if (file == nullptr) {
        std::cerr << "Failed to open file." << std::endl;
        // Signal to client via shared memory that file could not be opened
        strncpy(sharedData, "!-3", SHM_SIZE);
        return 1;
    }

    // Print each line of the file line by line to the stdout
    char line[256];
    for (int i = 0; i < lines_count; i++) {
        if (fgets(line, sizeof(line), file) != nullptr) {
            // Add a delimiter to the end of the line
            line[strlen(line) - 1] = '!';
            // Add the newline character to the end of the line 
            line[strlen(line)] = '\n';
            std::cout << line;
            // Copy the line to the shared memory
            strncpy(sharedData, line, SHM_SIZE);
            cout << "Data written to shared memory: " << line << endl;
        }
    }

    // Detach from the shared memory segment
    if (shmdt(sharedData) == -1) {
        std::cerr << "Failed to detach from shared memory segment." << std::endl;
        return 1;
    }

    


    return 0;
}