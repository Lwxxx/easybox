#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ddd_socket.h"
#include "component.h"

#define COMMAND_NAME "command"


void* command_s(void* arg)
{
	FILE* cmd_fp;
	char recv_buf[BUF_MAX] = {0};
	char reply_buf[BUF_MAX] = {0};
	struct sockaddr_in addr;
	socklen_t addr_len = (socklen_t)sizeof(addr);
	int command_sock = -1;

	command_sock = ddd_init_udp_socket(COMMAND_NAME, DDD_PORT_ANY, &addr);
	if (-1 == command_sock)
	{
		fprintf(stderr, "[command] init socked failed\n");
		return NULL;
	}

	for (;;)
	{
		memset(recv_buf, 0, BUF_MAX);
		recvfrom(command_sock, recv_buf, BUF_MAX, 0, (struct sockaddr *)&addr, &addr_len);
		printf("[command] recv: %s\n", recv_buf);

		cmd_fp = popen(recv_buf, "r");
		if (NULL == cmd_fp)
		{
			fprintf(stderr, "run command failed\n");
			continue;
		}
		else
		{
			while (NULL != fgets(reply_buf, BUF_MAX, cmd_fp))
			{
				sendto(command_sock, reply_buf, strlen(reply_buf), 0,
					   (struct sockaddr *)&addr, addr_len);
				memset(reply_buf, 0, BUF_MAX);
			}

			/* EOF */
			sendto(command_sock, NULL, 0, 0, (struct sockaddr *)&addr, addr_len);
		}

		pclose(cmd_fp);
	}

	close(command_sock);
	return NULL;
}
