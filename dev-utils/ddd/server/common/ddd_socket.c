#include <string.h>
#include <errno.h>

#include "ddd_socket.h"

GLOBAL int ddd_init_udp_socket(char* name, int port, struct sockaddr_in* addr)
{
	int ret  = 0;
	int sock = -1;
	int component_port = 0;
	socklen_t addr_len = (socklen_t)sizeof(addr);
	struct component* component = NULL;
	struct sockaddr_in binded_addr;

	bzero(addr, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	addr->sin_port = htons(port);

	// create socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sock)
	{
		fprintf(stderr, "[%s]: create socket failed, %s\n", name, strerror(errno));
		return -1;
	}

	// bind
	ret = bind(sock ,(struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if (-1 == ret)
	{
		fprintf(stderr, "[%s]: bind failed, %s\n", name, strerror(errno));
		close(sock);
		return -1;
	}

	// get binded port
	ret = getsockname(sock, (struct sockaddr *)&binded_addr, &addr_len);
	if (-1 == ret)
	{
		fprintf(stderr, "[%s]: getsockname failed, %s\n", name, strerror(errno));
		close(sock);
		return -1;
	}

	component_port = ntohs(binded_addr.sin_port);
	component = find_component_by_name(name);
	if (NULL == component)
	{
		fprintf(stderr, "[%s]: can not find component!\n", name);
		close(sock);
		return -1;
	}
	component->port = component_port;

	return sock;
}
