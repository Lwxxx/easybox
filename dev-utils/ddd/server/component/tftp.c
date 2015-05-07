#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>

#include "eb_global.h"
#include "ddd_socket.h"
#include "component.h"

enum
{
	TFTP_OP_RRQ = 1,
	TFTP_OP_WRQ,
	TFTP_OP_DATA,
	TFTP_OP_ACK,
	TFTP_OP_ERROR
};

enum
{
	TFTP_ERR_NOT_DEFINE = 0,
	TFTP_ERR_FILE_NOT_FOUND,
	TFTP_ERR_ACC_VIOLA,
	TFTP_ERR_DISKFULL,
	TFTP_ERR_INV_OPCODE,
	TFTP_ERR_UNKNOWN_TID,
	TFTP_ERR_FILE_EXISTS,
	TFTP_ERR_NO_SUCH_USER,
};


LOCAL char errcode_str[][32] =
{
	"",
	"file not found",
	"access violation",
	"disk full",
	"bad operation",
	"unknown transfer id",
	"file already exists",
	"no such user"
};


#define TFTP_PORT       69
#define TFTP_DATA_BUF   512
#define TFTP_TIMEOUT    2
#define TFTP_RETRY_MAX  5
#define TFTP_NAME       "tftp"
#define TFTP_ASCII_MODE "netascii"
#define TFTP_OCTET_MODE "octet"
#define TFTP_MAIL_MODE  "mail"

LOCAL struct sockaddr_in addr;
LOCAL int tftp_sock = -1;
LOCAL socklen_t addr_len = (socklen_t)sizeof(addr);
LOCAL FILE* write_fp = NULL;
LOCAL FILE* read_fp  = NULL;
LOCAL int curr_write_block = 0;
LOCAL int curr_read_block  = 0;
LOCAL char read_buf[TFTP_DATA_BUF] = {0};
LOCAL int read_data_len = 0;
LOCAL int retry_times = 0;
LOCAL int send_file_end = 0;

LOCAL void send_tftp_ack(uint16_t number)
{
	char     buf[4]   = {0};
	uint16_t opcode   = htons(TFTP_OP_ACK);
	uint16_t block_no = htons(number);

	if (-1 == tftp_sock)
	{
		fprintf(stderr, "tftp socked is invalid\n");
		return;
	}

	memcpy(buf, &opcode, 2);
	memcpy(buf+2, &block_no, 2);

	sendto(tftp_sock, buf, 4, 0, (struct sockaddr *)&addr, addr_len);
}

LOCAL void send_tftp_data()
{
	char buf[TFTP_DATA_BUF + 4] = {0};
	uint16_t opcode   = htons(3);
	uint16_t block_no = htons(curr_read_block);

	if (NULL == read_fp || -1 == tftp_sock)
	{
		return;
	}

	memcpy(buf, &opcode, 2);
	memcpy(buf+2, &block_no, 2);
	memcpy(buf+4, read_buf, TFTP_DATA_BUF);

	sendto(tftp_sock, buf, read_data_len + 4, 0, (struct sockaddr *)&addr, addr_len);
}

LOCAL void tftp_retransmit()
{
	if (retry_times >= TFTP_RETRY_MAX)
	{
		retry_times = 0;
		printf("[tftp] send failed\n");
		return;
	}

	printf("[tftp] time out, retransmit data\n");
	retry_times += 1;
	send_tftp_data();
	alarm(TFTP_TIMEOUT);
}

LOCAL void send_tftp_error(int err_code)
{
	char* buf = NULL;
	char* err_msg_str = NULL;
	int   err_msg_len = 0;

	uint16_t opcode = htons(TFTP_OP_ERROR);
	uint16_t error = htons(err_code);

	if (-1 ==tftp_sock ||
		err_code < TFTP_ERR_NOT_DEFINE || err_code > TFTP_ERR_NO_SUCH_USER)
	{
		return;
	}

	err_msg_str = errcode_str[err_code];
	err_msg_len = strlen(err_msg_str);
	buf = (char*)calloc((err_msg_len + 5), 1);

	memcpy(buf, &opcode, 2);
	memcpy(buf+2, &errno, 2);
	memcpy(buf+4, err_msg_str, err_msg_len);

	printf("[tftp] send error: %d\n", err_code);
	sendto(tftp_sock, buf, err_msg_len + 5, 0, (struct sockaddr *)&addr, addr_len);
}

LOCAL void handle_tftp_rrq(char* buf)
{
	char filename[EB_PATH_MAX] = {0};
	char mode[12] = {0};
	int  filename_len = 0;

	// get filename
	strncpy(filename, buf+2, EB_PATH_MAX);
	filename_len = strlen(filename);

	// get mode
	strncpy(mode, buf+2+filename_len+1, 12);

	// only support octet mode, netascii and mail are both useless
	if (0 != strcasecmp(TFTP_ASCII_MODE, mode) &&
		0 != strcasecmp(TFTP_OCTET_MODE, mode)  &&
		0 != strcasecmp(TFTP_MAIL_MODE, mode))
	{
		send_tftp_error(TFTP_ERR_NOT_DEFINE);
		return;
	}

	read_fp = fopen(filename, "rb");
	if (NULL == read_fp)
	{
		send_tftp_error(TFTP_ERR_FILE_NOT_FOUND);
		return;
	}

	curr_read_block = 1;
	retry_times     = 0;
	send_file_end   = 0;

	memset(read_buf, 0, TFTP_DATA_BUF);
	read_data_len = fread(read_buf, 1, TFTP_DATA_BUF, read_fp);
	send_tftp_data();

	if (read_data_len < TFTP_DATA_BUF)
	{
		fclose(read_fp);
		send_file_end = 1;
	}

	signal(SIGALRM, tftp_retransmit);
	alarm(TFTP_TIMEOUT);
}

LOCAL void handle_tftp_wrq(char* buf)
{
	char filename[EB_PATH_MAX] = {0};
	char mode[12] = {0};
	int  filename_len = 0;

	// get filename
	strncpy(filename, buf+2, EB_PATH_MAX);
	filename_len = strlen(filename);

	// get mode
	strncpy(mode, buf+2+filename_len+1, 12);

	// only support octet mode, netascii and mail are both useless
	if (0 != strcasecmp(TFTP_ASCII_MODE, mode) &&
		0 != strcasecmp(TFTP_OCTET_MODE, mode)  &&
		0 != strcasecmp(TFTP_MAIL_MODE, mode))
	{
		send_tftp_error(TFTP_ERR_FILE_NOT_FOUND);
		return;
	}

	// open file for write
	write_fp = fopen(filename, "w");
	if (NULL == write_fp)
	{
		send_tftp_error(TFTP_ERR_ACC_VIOLA);
		return;
	}

	// send reply
	send_tftp_ack(0);
}

LOCAL void handle_tftp_data(char* buf, int len)
{
	uint16_t block_no;
	char* data;

	if (NULL == buf || len < 4)
	{
		send_tftp_error(TFTP_ERR_NOT_DEFINE);
		return;
	}

	memcpy(&block_no, buf+2, 2);
	block_no = ntohs(block_no);

	if (block_no != (curr_write_block + 1))
	{
		fprintf(stderr, "[tftp] invalid block number: %d\n", block_no);
		//+?? just ignore
		return;
	}

	curr_write_block = block_no;
	data = buf + 4;
	fwrite(data, len - 4, 1, write_fp);
	send_tftp_ack(curr_write_block);

	if (512 != (len - 4)) // final part
	{
		fclose(write_fp);
		curr_write_block = 0;
	}
}

LOCAL void handle_tftp_ack(char* buf)
{
	uint16_t opcode   = 0;
	uint16_t block_no = 0;

	if (1 == send_file_end)
	{
		printf("[tftp] send file success\n");
		alarm(0);
		send_tftp_ack(curr_read_block + 1);
		return;
	}

	retry_times = 0;

	memcpy(&opcode, buf, 2);
	opcode = ntohs(opcode);

	memcpy(&block_no, buf+2, 2);
	block_no = ntohs(block_no);

	if (block_no == curr_read_block)
	{
		curr_read_block += 1;
		memset(read_buf, 0, TFTP_DATA_BUF);
		read_data_len = fread(read_buf, 1, TFTP_DATA_BUF, read_fp);
		send_tftp_data();

		if (read_data_len < TFTP_DATA_BUF)
		{
			fclose(read_fp);
			send_file_end = 1;
		}
		else
		{
			alarm(TFTP_TIMEOUT);
		}
	}
	else
	{
		printf("[tftp] invalid block number\n");
	}
}

LOCAL void handle_tftp_error(char* buf)
{
	fprintf(stderr, "%s", buf + 4);
}

LOCAL void handle_tftp_packet(char* buf, int len)
{
	uint16_t opcode  = 0;

	if (NULL == buf)
	{
		return;
	}

	memcpy(&opcode, buf, 2);
	opcode = ntohs(opcode);

	switch(opcode)
	{
	case TFTP_OP_RRQ:
		printf("[tftp] RRQ\n");
		handle_tftp_rrq(buf);
		break;
	case TFTP_OP_WRQ:
		printf("[tftp] WRQ\n");
		handle_tftp_wrq(buf);
		break;
	case TFTP_OP_DATA:
		// printf("[tftp] DATA\n");
		handle_tftp_data(buf, len);
		break;
	case TFTP_OP_ACK:
		// printf("[tftp] ACK\n");
		handle_tftp_ack(buf);
		break;
	case TFTP_OP_ERROR:
		printf("[tftp] ERROR\n");
		handle_tftp_error(buf);
		break;
	default:
		break;
	}
}

void* tftp_s(void* arg)
{
	char recv_buf[BUF_MAX] = {0};
	int  recv_len = 0;
	socklen_t addr_len = (socklen_t)sizeof(addr);

	tftp_sock = ddd_init_udp_socket(TFTP_NAME, TFTP_PORT, &addr);
	if (-1 == tftp_sock)
	{
		fprintf(stderr, "[tftp] init socket failed\n");
		return NULL;
	}

	for (;;)
	{
		memset(recv_buf, 0, BUF_MAX);
		recv_len = recvfrom(tftp_sock, recv_buf, BUF_MAX, 0, (struct sockaddr*)&addr, &addr_len);
		if (-1 != recv_len)
		{
			handle_tftp_packet(recv_buf, recv_len);
		}
	}

	close(tftp_sock);
}
