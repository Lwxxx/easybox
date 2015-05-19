#ifndef __NETFIFO_H__
#define __NETFIFO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>


#define NFF_BUF_SIZE 2048
#define NFF_DFT_PORT 7788
#define NFF_BACKLOG  5

#ifndef PIPE_BUF
#define PIPE_BUF 4096
#endif // PIPE_BUF

#define UGO_RW S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
//#define NFF_FIFO "/var/run/nff.fifo"
#define NFF_FIFO "./test.fifo"


#define D(fmt, ...) do {							\
    printf("[DBG] | %d | <%s> : "fmt"\n",           \
			(int)time(0), __func__, ##__VA_ARGS__);	\
} while (0)

#define E(fmt, ...) do {                                            \
    fprintf(stderr, "[ERR] | %d | <%s> : "fmt" | {%s}\n",           \
        (int)time(0), __func__, ##__VA_ARGS__, strerror(errno));	\
} while (0)

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus	


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __NETFIFO_H__
