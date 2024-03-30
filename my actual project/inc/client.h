// Copyright D. L. Devito 2024
#ifndef CLIENT_H_
#define CLIENT_H_
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <semaphore.h>
#include <thread> // Only for sleeping while creating and debugging.
#include <chrono>
#include <vector>
#include <string>
#include "./calculator.h"

#define SHM_SIZE 0x1D0900

using std::cout; 
using std::endl;
using std::string;
using std::vector;

using namespace calculator;

// Structure to use as memory buffer.
struct shmbuf {
    char buf[0x900000];
};

/* This function accepts a line and converts it into a vector containing the values and operations found within it. This is required to pass to calculate() in order to calculate infix notation. */
vector <string> convert_line_to_string_vector(string line);

#endif  // CLIENT_H_