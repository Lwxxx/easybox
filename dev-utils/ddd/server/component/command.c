#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eb_global.h"
#include "ddd_socket.h"
#include "component.h"

#define COMMAND_NAME "command"

LOCAL void run_command(char* cmd, char* result)
{
	FILE* cmd_fp = NULL;
	char  line_buf[BUF_MAX] = {0};

	if (NULL == cmd || NULL == result)
	{
		return;
	}

	cmd_fp = popen(cmd, "r");
	if (NULL == cmd_fp)
	{
		fprintf(stderr, "run command failed\n");
	}
	else
	{
		while (NULL != fgets(line_buf, BUF_MAX, cmd_fp))
		{
			if ((strlen(result) + strlen(line_buf) > BUF_MAX))
			{
				break;
			}
			else
			{
				strcat(result, line_buf);
				memset(line_buf, 0, BUF_MAX);
			}
		}
	}

	pclose(cmd_fp);
}

GLOBAL void* command_s(void* arg)
{
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
		memset(reply_buf, 0, BUF_MAX);
		recvfrom(command_sock, recv_buf, BUF_MAX, 0, (struct sockaddr *)&addr, &addr_len);
		printf("[command] recv: %s\n", recv_buf);
		run_command(recv_buf, reply_buf);
		sendto(command_sock, reply_buf, strlen(reply_buf), 0,
			   (struct sockaddr *)&addr, addr_len);
	}

	close(command_sock);
	return NULL;
}
