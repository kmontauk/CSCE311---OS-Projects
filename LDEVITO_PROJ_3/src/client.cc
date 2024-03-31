// Copyright D. L. Devito 2024
#include <client.h>

// Declaring required structures.
struct timespec time_1;

struct shmbuf* shmp;

// Define global variables for threading
int current_thread = 0;
int64_t thread_sum[4];  // This is for the sum of each thread
int thread_lines[4];  // This is for the number of lines each thread processed.

int lines_count = 0;


/* This function accepts a line and converts it into a vector containing the 
values and operations found within it. This is required to pass to calculate() 
in order to calculate infix notation. */
vector <string> convert_line_to_string_vector(string line) {
    vector<string> string_vec;
    string temp = "";
    int length = line.length();
    for (int i = 0; i <= length; i++) {
        if (line[i] == ' ' || i == length) {
            string_vec.push_back(temp);
            temp = "";
        } else {
            temp += line[i];
        }
    }
    return string_vec;
}

/* This function accepts a void pointer argument, which for use by client.cc,
   is always a ThreadArgs* pointing to a ThreadArgs structure. The structure
   contains the shared memory buffer and thread ID of the current thread 
   executing the code. It totals 1/4th of the lines in a round robin fashion
   across all lines and returns null. */
void* calculate_thread(void* arg) {
    // Cast the argument to a ThreadArgs pointer
    ThreadArgs* args = reinterpret_cast<ThreadArgs*>(arg);
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

    int64_t sum = 0;
    int response_length = response.length();
    int lines_processed = 0;
    std::vector<std::string> lines(lines_count);
    string line = "";

    // This adds each line into the vector 'lines'
    int j = 0;
    for (int i = 1; i < response_length; i++) {
        // cout << "hi";
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
            sleep(3);
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
    shmp = reinterpret_cast<shmbuf*>(mmap(NULL, sizeof(*shmp),
     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (shmp == MAP_FAILED) {
        return 1;
    }

    // Create the file name, path, and lines_str
    const char* file_path = argv[1];
    std::string lines_str = "!";
    lines_count = std::stoi(argv[2]);
    lines_str += std::to_string(lines_count);

    // Wait for the server to signal that it is ready
    while (server_semaphore == 0) {}

    // Copy the file path and lines_str to the shared memory
    strncpy(shmp->buf, file_path, SHM_SIZE);  // file name
    strncat(shmp->buf, lines_str.c_str(),
    SHM_SIZE - strlen(shmp->buf) - 1);  // lines_count


    // Signal the server that the shared memory is ready
    sem_post(client_semaphore);

    // Wait for the server to signal that it is done
    sem_wait(server_semaphore);

    // I'm not 100% sure why it doesn't block here ^^^, but it doesn't.
    // I think it's because the server_semaphore is posted in the server.cc.
    int value;
    sem_getvalue(server_semaphore, &value);
    while (value == 0) {
        sem_getvalue(server_semaphore, &value);
    }

    // Save contents of shared memory for error checking.
    string response = shmp->buf;
    // If the server returns an invalid file, exit the program
    if (response[0] == '!' && response[1] == '!') {
        std::cerr << "!! INVALID FILE !!" << endl;
        // Attempt shared memory cleanup for clean client shutdown.
        try {
            // Unmap
            munmap(shmp, SHM_SIZE);
            // Close the shared memory
            close(shm_fd);
            // Remove the shared memory
            shm_unlink(shm_name);
            exit(0);
        } catch (const std::exception& e) {
            exit(1);
        }
        exit(0);
    }

    // Using pthreads to do the same thing we do later with a single thread
    pthread_t threads[4];
    // ThreadArgs structure holding shmp and thread_id
    ThreadArgs thread_args[4];

    // Assign values to data members of structure for each thread
    for (int i = 0; i < 4; i++) {
        thread_args[i].shmp = shmp;
        thread_args[i].thread_id = i;
        pthread_create(&threads[i], NULL, calculate_thread, &thread_args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    int64_t total = 0;
    // Sum the sums of the threads
    for (int i = 0; i < 4; i++) {
        cout << "THREAD " << i << ": " << thread_lines[i] << " LINES, "
        << thread_sum[i] << endl;
        total += thread_sum[i];
    }

    // Finally :)
    cout << "SUM: " << total << endl;

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
    return 0;
}
