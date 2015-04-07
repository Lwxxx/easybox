#ifndef __DDD_SOCKET_H__
#define __DDD_SOCKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "eb_global.h"
#include "component.h"

#define DDD_PORT_ANY 0
#define BUF_MAX 1024


#ifdef __cplusplus
extern "C"
{
#endif

GLOBAL int ddd_init_udp_socket(char* name, int port, struct sockaddr_in* addr);

#ifdef __cplusplus
}
#endif

#endif // __DDD_SOCKET_H__
