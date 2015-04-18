#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>

#include "eb_global.h"


#define DFT_SYNC_INTERVAL 1
#define DFT_RBUF_SIZE     40960  // 40KB

struct ring_buffer
{
	char* buf;
	int   len;
	int   start_pos;
	int   cur_pos;
	int   modify_flag;
	int   full_flag;
};

LOCAL struct ring_buffer rbuf;
LOCAL char out_file[EB_PATH_MAX] = {0};
LOCAL int sync_interval = DFT_SYNC_INTERVAL;


/**
 * @brief read data from standart input (mostly by pipe)
 */
LOCAL int ring_read(char* ch, int size)
{
	return fread(ch, size, 1, stdin);
}

/**
 * @brief wirte data to ring buffer
 */
LOCAL int ring_write(char* data, int size)
{
	int tmp = 0;

	if ((rbuf.len - rbuf.cur_pos) >= size)
	{
		memcpy(rbuf.buf + rbuf.cur_pos, data, size);
		rbuf.cur_pos = rbuf.cur_pos + size;
		if (1 == rbuf.full_flag)
		{
			rbuf.start_pos = rbuf.cur_pos;
		}
	}
	else
	{
		tmp = rbuf.len - rbuf.cur_pos;
		memcpy(rbuf.buf + rbuf.cur_pos, data, tmp);
		memcpy(rbuf.buf, data + tmp, size - tmp);
		rbuf.cur_pos = size - tmp;
		rbuf.start_pos = rbuf.cur_pos;
		rbuf.full_flag = 1;
	}

	rbuf.modify_flag = 1;
	return size;
}

/**
 * @brief write date from ring buffer to real file
 */
LOCAL void ring_sync()
{
	int fd = -1;

	if (1 == rbuf.modify_flag)
	{
		// standard I/O Library was not safe in signal handler,
		// use open/read/write directly.
		fd = open(out_file, O_CREAT|O_WRONLY|O_SYNC|O_TRUNC,
				  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (-1 == fd)
		{
			printf("open file %s filed\n", out_file);
		}
		assert(fd != -1);

		if (0 == rbuf.start_pos)
		{
			write(fd, rbuf.buf, rbuf.cur_pos);
		}
		else
		{
			write(fd, rbuf.buf + rbuf.start_pos, rbuf.len - rbuf.start_pos);
			write(fd, rbuf.buf, rbuf.start_pos);
		}

		//+TODO write data to file
		rbuf.modify_flag = 0;
		close(fd);
	}

	alarm(sync_interval);
}

/**
 * @brief show usage infomation
 */
LOCAL void show_usage(void)
{
	char usage[] = "usage: COMMAND | ring -f FILE [-l] LEN\n"
		"\noptions:\n"
		"\t  -f file   set output file\n"
		"\t  -l lenth  set buffer length\n"
		"\t  -h        show help infomation\n";

	printf("%s\n", usage);
	exit(1);
}

int main(int argc, char *argv[])
{
	char ch       = 0;
	int  buf_size = DFT_RBUF_SIZE;
	int  arg_val  = 0;

	if (argc < 2)
	{
		show_usage();
	}

	// parse command line options
	while (EOF != (arg_val = getopt(argc, argv, "o:l:h")))
	{
		switch(arg_val)
		{
		case 'o':
			sprintf(out_file, optarg, EB_PATH_MAX);
			break;
		case 'l':
			buf_size = atoi(optarg);
			break;
		case 'h': // fall through
		default:
			show_usage();
			break;
		}
	}

	// init ring buffer
	memset(&rbuf, 0, sizeof(struct ring_buffer));
	rbuf.len = buf_size;
	rbuf.buf = (char*)malloc(rbuf.len);
	assert(rbuf.buf);

	// set sync timer
	signal(SIGALRM, ring_sync);
	alarm(sync_interval);

	// read data from stdin and redict to ring buffer
	while (ring_read(&ch, 1))
	{
		ring_write(&ch, 1);
	}

	if (rbuf.buf) free(rbuf.buf);

	return 0;
}
