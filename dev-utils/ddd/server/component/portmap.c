#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ddd_socket.h"
#include "component.h"

#define PORTMAP_PORT 36925
#define PORTMAP_NAME "portmap"

void* portmap_s(void* arg)
{
	char buf[BUF_MAX] = {0};
	char port_str[8]  = {0};
	struct component* component = NULL;
	struct sockaddr_in addr;
	socklen_t addr_len = (socklen_t)sizeof(addr);
	int component_port = 0;
	int portmap_sock = -1;

	portmap_sock = ddd_init_udp_socket(PORTMAP_NAME, PORTMAP_PORT, &addr);
	if (-1 == portmap_sock)
	{
		fprintf(stderr, "[portmap] init socket failed\n");
		return NULL;
	}

	for(;;)
	{
		memset(buf, 0, BUF_MAX);
		memset(port_str, 0, 8);
		recvfrom(portmap_sock, buf, BUF_MAX, 0, (struct sockaddr *)&addr, &addr_len);
		printf("[portmap] recv: %s\n", buf);
		component = find_component_by_name(buf);
		if (NULL != component)
		{
			printf("%s : %d\n", component->name, component->port);
			component_port = htons(component->port);
		}
		else
		{
			fprintf(stderr, "[portmap] can not find component: %s\n", buf);
			component_port = 0;
		}
		sprintf(port_str, "%d", component_port);
		sendto(portmap_sock, port_str, strlen(port_str), 0,
			   (struct sockaddr *)&addr, addr_len);
	}

	if (-1 != portmap_sock)
	{
		close(portmap_sock);
	}

	return NULL;
}
