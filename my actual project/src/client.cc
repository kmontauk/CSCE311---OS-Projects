// Copyright D. L. Devito 2024
#include <client.h>
#include <pthread.h>
#include <time.h>

struct timespec time_1;

struct shmbuf* shmp;

// Define a mutex and condition variable
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Define global variables for threading
int current_thread = 0;

long long thread_sum[4];  // This is for the sum of each thread
int thread_lines[4];  // This is for the number of lines processed by each thread

string response = "";
int lines_count = 0;

struct ThreadArgs {
    shmbuf* shmp;
    int thread_id;
};

/* This function accepts a line and converts it into a vector containing the 
values and operations found within it. This is required to pass to calculate() 
in order to calculate infix notation. */
vector <string> convert_line_to_string_vector(string line) {
    vector<string> string_vec;
    string temp = "";
    for (int i = 0; i <= line.length(); i++) {
        if (line[i] == ' ' || i == line.length()) {
            string_vec.push_back(temp);
            temp = "";
        } else {
            temp += line[i];
        }
    }
    return string_vec;
}

void* calculate_thread(void* arg) {
    // Cast the argument to a ThreadArgs pointer
    ThreadArgs* args = (ThreadArgs*)arg;
    // Extract the shared memory object and thread ID
    shmbuf* shmp = args->shmp;
    int thread_id = args->thread_id;
    // I did this as opposed to just reading the global variable for 'response'
    // because I wanted to follow the guidelines as strictly as possible. But,
    // this seems more resource intensive (4x identical response variables).
    std::string response(shmp->buf);
    
    // For use in string vector once it is completely filled.
    int starting_index = 0;
    if (thread_id ==  0) starting_index = 0;
    else if (thread_id == 1) starting_index = 1;
    else if (thread_id == 2) starting_index = 2;
    else if (thread_id == 3) starting_index = 3;

    long long sum = 0;
    int response_length = response.length();
    int lines_processed = 0;
    std::vector<std::string> lines(lines_count);
    string line = "";

    
  
    // This adds each line into the vector 'lines'
    int j = 0;
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

    // This calculates the sum of 1/4th of the lines of the vector.
    for (int i = starting_index; i < lines_count; i+=4) {
        vector <string> string_vec = convert_line_to_string_vector(lines[i]);
        size_t size = string_vec.size();
        int size_int = static_cast<int>(size);
        sum += calculate(size_int, string_vec);
        lines_processed++;
    }

    // Set the correct thread_sum global variable for further summing
    // within main() later
    switch (thread_id) {
    case 0:
        thread_sum[0] = sum;
        thread_lines[0] = lines_processed;
        break;
    case 1:
        thread_sum[1] = sum;
        thread_lines[1] = lines_processed;
        break;
    case 2:
        thread_sum[2] = sum;
        thread_lines[2] = lines_processed;
        break;
    case 3:
        thread_sum[3] = sum;
        thread_lines[3] = lines_processed;
        break;
    default:
        break;
    }

    return NULL;
}



int main(int argc, char* argv[]) {
    // Input checking
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << 
        " <file path> <number of lines>" << std::endl;
        return 1;
    }
    cout << "SHARED MEMORY ALLOCATED: " << SHM_SIZE << " BYTES" << endl;
    // open existing semaphores located on the server
    sem_t* client_semaphore = sem_open("/client_semaphore", 0);
    sem_t* server_semaphore = sem_open("/server_semaphore", 0); 
    if (client_semaphore == SEM_FAILED || server_semaphore == SEM_FAILED) {
        cout << "Failed to open semaphores."  <<
        " Attempting to open again, server may be off." << endl;
        while (true) {
            // Forgive my injecting humor, this assignment hurt.
            printf("open. up. the server. stop. having it. be closed. \n");
            std::this_thread::sleep_for(std::chrono::seconds(3));
            client_semaphore = sem_open("/client_semaphore", 0);
            server_semaphore = sem_open("/server_semaphore", 0); 
            if (client_semaphore != SEM_FAILED 
            && server_semaphore != SEM_FAILED) {
                break;
            }
        }
    }

    // Generate a unique name for the shared memory object
    const char* shm_name = "/client_shared_memory";

    // Create the shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        return 1;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        return 1;
    }

    // Map the shared memory object into the process's address space
    shmp = (shmbuf*) mmap(NULL, sizeof(*shmp),
     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shmp == MAP_FAILED) {
        return 1;
    }

    // Wait for the server to signal that it is ready
    while(server_semaphore == 0) {}

    // Create the file name, path, and lines_str
    const char* file_path = argv[1];
    std::string lines_str = "!";
    lines_count = std::stoi(argv[2]);
    lines_str += std::to_string(lines_count);

    // Copy the file path and lines_str to the shared memory
    strncpy(shmp->buf, file_path, SHM_SIZE); // file name
    strncat(shmp->buf, lines_str.c_str(), 
    SHM_SIZE - strlen(shmp->buf) - 1); // lines_count


    // Signal the server that the shared memory is ready
    sem_post(client_semaphore);


    // Sleep for 1 second to allow the server to process the shared memory
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    time_1.tv_sec = 1;
    time_1.tv_nsec = 0; // additional nanoseconds
    nanosleep(&time_1, NULL);
    
    // Print contents of shared memory
    response = shmp->buf; 
    //std::cout << "Server response read from shared memory: " << response << std::endl;

    // Using pthreads to do the same thing we do later with a single thread
    pthread_t threads[4];
    // ThreadArgs structure holding shmp and thread_id
    ThreadArgs thread_args[4];
    
    for (int i = 0; i < 4; i++) {
        thread_args[i].shmp = shmp;
        thread_args[i].thread_id = i;
        pthread_create(&threads[i], NULL, calculate_thread, &thread_args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    long long total = 0;
    // Sum the sums of the threads
    for (int i = 0; i < 4; i++) {
        cout << "THREAD " << i << ": " << thread_lines[i] << " LINES, "
        << thread_sum[i] << endl;
        total += thread_sum[i];
    }

    cout << "SUM: " << total << endl;




    // Single threaded version
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
    /**
    cout << "Lines vector contents: " << endl;
    for (int i = 0; i <= (int)lines.size() - 1; i++) {
        cout << lines[i] << endl;
    } **/

    // Unmap the shared memory object
    if (munmap(shmp, SHM_SIZE) == -1) {
        return 1;
    }

    // Close the shared memory object
    if (close(shm_fd) == -1) {
        return 1;
    }

    // Remove the shared memory object
    if (shm_unlink(shm_name) == -1) {
        return 1;
    }
    
    // OKAY. Shared memory fun (not fun) is over. 

    // Now, we can start the calculation process.
    long long sum = 0;
    for (int i = 0; i < lines_count; i++) {
        vector <string> string_vec = convert_line_to_string_vector(lines[i]);
        size_t size = string_vec.size();
        int size_int = static_cast<int>(size);
        sum += calculate(size_int, string_vec);
    }
    cout << "SUM: " << sum << endl;

    
    return 0;
}



