#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>

#include "netfifo.h"

#define NFFC_RECONN_MAX 5
#define HOST_MAX 64

struct nffc_handle {
	int   sock;
	struct sockaddr_in srv_addr;
	int   retry;
	pthread_mutex_t lock;
};

static short npd_port;
static char  npd_hostname[HOST_MAX];
static struct nffc_handle nffc;
static char recv_buf[NFF_BUF_SIZE] = {0};


static void show_usage(void)
{
	char usage[] = "Usage: nffc REMOTE [-p port] [-o file]\n\n";

	printf("%s", usage);
	exit(1);
}

static int nffc_sock_init(void)
{
	struct hostent *server_host;

	// get server ip by host
	if (NULL == (server_host = gethostbyname(npd_hostname))) {
		E("gethostbyname");
		return -1;
	}

	// set srv_address ip:port
	bzero(&(nffc.srv_addr),sizeof(nffc.srv_addr));
	nffc.srv_addr.sin_family      = AF_INET;
	nffc.srv_addr.sin_addr.s_addr = ((struct in_addr *)(server_host->h_addr))->s_addr;
	nffc.srv_addr.sin_port=htons(npd_port);

	// create socket
	nffc.sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == nffc.sock) {
		E("create socket failed");
		return -1;
	}

	// connect to server
	if (connect(nffc.sock, (struct sockaddr *)&(nffc.srv_addr), sizeof(nffc.srv_addr)) == -1) {
		E("connect to server failed");
		return -1;
	}
}


int main(int argc, char** argv) {
	int arg_val;
	struct sigaction sigact;

	if (argc < 2)
	{
		show_usage();
	}

	npd_port = NFF_DFT_PORT;
	strncpy(npd_hostname, argv[1], HOST_MAX);

	// parser command line options
	while (EOF != (arg_val = getopt(argc, argv, "p:o:h"))) {
		switch (arg_val) {
		case 'p':
			break;
		case 'o':
			break;
		case 'h':
			show_usage();
			break;
		default:
			break;
		}
	}

	// init socket
	if (-1 == nffc_sock_init()) {
		E("initialize socket failed, exit");
		exit(1);
	}

	// recv data from server and output
	for (;;) {
		memset(recv_buf, 0, NFF_BUF_SIZE);
		if (recv(nffc.sock, recv_buf, NFF_BUF_SIZE, 0) > 0) {
			puts(recv_buf);
		} else {
			E("connection was close by server");
			exit(-1);
		}
	}

	close(nffc.sock);

	return 0;
}
