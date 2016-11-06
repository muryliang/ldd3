#ifndef HEAD_H
#define HEAD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define per(fmt,...) do { \
printf(fmt,##__VA_ARGS__); \
exit(1); \
} while(0)

#endif
