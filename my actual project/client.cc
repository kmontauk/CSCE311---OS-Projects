#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <semaphore.h>
#include <thread> // Only for sleeping while creating and debugging.
#include <chrono>
#include "./client.h"
#include <vector>
#include <string>
#include "./calculator.h"
using std::cout; 
using std::endl;
using std::string;
using std::vector;
using namespace calculator;

#define SHM_SIZE 0x400

struct shmbuf* shmp;

vector <string> convert_line_to_string_vector(string line) {
    vector<string> string_vec;
    string temp = "";
    //cout << "Line: " << line << endl;
    for (int i = 0; i <= line.length(); i++) {
        if (line[i] == ' ' || i == line.length()) {
            //printf("Pushing %s to string_vec\n", temp.c_str());
            string_vec.push_back(temp);
            temp = "";
        } else {
            temp += line[i];
        }
    }
    return string_vec;
}


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
    const char* shm_name = "/client_shared_memory";

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
    while(server_semaphore == 0) {}

    // Create the file name, path, and lines_str
    const char* file_path = argv[1];
    std::string lines_str = "!";
    int lines_count = std::stoi(argv[2]);
    lines_str += std::to_string(lines_count);

    // Copy the file path and lines_str to the shared memory
    strncpy(shmp->buf, file_path, SHM_SIZE); // file name
    strncat(shmp->buf, lines_str.c_str(), SHM_SIZE - strlen(shmp->buf) - 1); // lines_count


    std::cout << "File name, path, and lines_str have been written to shared memory." << std::endl;

    std::string data(shmp->buf);

    // Print the contents of the shared memory
    std::cout << "Data read from shared memory: " << data << std::endl;

    // Signal the server that the shared memory is ready
    sem_post(client_semaphore);

    // Sleep for 2 seconds to allow the server to process the shared memory
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Print contents of shared memory
    std::string response(shmp->buf);
    std::cout << "Server response read from shared memory: " << response << std::endl;

    // Process the server response, converting it to a string vector.
    std::vector<std::string> lines(lines_count); // Initialize the lines vector
    // Let i = the current index of the response string, j = current line
    int j = 0;
    int response_length = response.length();
    string line;
    for (int i = 1; i < response_length; i++) {
        //cout << "hi";
        if (response[i] == '!') {
            lines[j] = line;
            line = "";
            j++;
            continue;
        }
        line += response[i];
    }

    // Print the contents of the lines vector
    cout << "Lines vector contents: " << endl;
    for (int i = 0; i <= (int)lines.size() - 1; i++) {
        cout << lines[i] << endl;
    }

    // Unmap the shared memory object
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
    
    // OKAY. Shared memory garbage is finally over. 

    // Now, we can start the calculation process.
    // We need a function to convert a line in the string vector to a double vector of individual values.
    printf("Testing the conversion function...\n");
    vector <string> test = convert_line_to_string_vector(lines[0]);
    size_t size = test.size();
    int size_int = static_cast<int>(size);
    cout << calculate(size_int, test) << endl;


    int sum = 0;
    









    

    return 0;
}