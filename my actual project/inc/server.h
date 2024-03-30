// Copyright D. 'L.' Devito 2024
#ifndef SERVER_H_
#define SERVER_H_
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

#define SHM_SIZE 0x1D0900

using std::cout;
using std::endl;
using std::string;
using std::vector;

// Structure to use as memory buffer
struct shmbuf {
    char buf[0x900000];
};

/** This function destroys the semaphore when the server is terminated, or when errors are encountered. This is for redunancy, as well as for continuity for when running the client prior to the server, as the semaphores are always deleted and recreated at server startup anyway */
void destroySemaphore(int signal);

#endif  // SERVER_H_