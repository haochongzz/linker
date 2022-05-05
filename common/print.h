#include <stdio.h>

#define DEBUG(msg) printf("debug: %s", msg)

#define ERROR(msg) { \
    fprintf(stderr, "error: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
    exit(1); \
}