// Copyright D. 'L.' Devito 2024
#ifndef LDEVITO_PROJ_3_INC_SERVER_H_
#define LDEVITO_PROJ_3_INC_SERVER_H_

// C system files
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

// C++ system files
#include <csignal>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define SHM_SIZE 0x1D0900

using std::cout;
using std::endl;
using std::string;
using std::vector;

// Structure to use as memory buffer
struct shmbuf {
    char buf[0x900000];
};

/** This function destroys the semaphore when the server is terminated, or when
 *  errors are encountered. This is for redunancy, as well as for continuity
 * for when running the client prior to the server, as the semaphores are
 * always deleted and recreated at server startup anyway */
void destroySemaphore(int signal);

#endif  // LDEVITO_PROJ_3_INC_SERVER_H_
