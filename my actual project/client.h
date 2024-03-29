#include <vector>

#define SHM_SIZE 0x400
//#define BUF_SIZE 0x400

struct shmbuf {
    char buf[0x100][0x100];
};