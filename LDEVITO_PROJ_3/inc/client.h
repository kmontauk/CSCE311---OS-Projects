// Copyright D. L. Devito 2024
#ifndef LDEVITO_PROJ_3_INC_CLIENT_H_
#define LDEVITO_PROJ_3_INC_CLIENT_H_

// Other includes
#include <calculator.h>

// C system files
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

// C++ system files
#include <ctime>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#define SHM_SIZE 0x1D0900

using std::cout;
using std::endl;
using std::string;
using std::vector;

using calculator::calculate;

// Structure to use as memory buffer.
struct shmbuf {
    char buf[0x900000];
};

// Structure to pass as thread arguments.
struct ThreadArgs {
    shmbuf* shmp;
    int thread_id;
};

/* This function accepts a line and converts it into a vector containing the
   values and operations found within it. This is required to pass to 
   calculate() in order to calculate infix notation. */
vector <string> convert_line_to_string_vector(string line);

#endif  // LDEVITO_PROJ_3_INC_CLIENT_H__
