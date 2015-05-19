#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "netfifo.h"


static int client_sock_list[NFF_BACKLOG];
static pthread_t fifo_thread;
static pthread_mutex_t nff_lock = PTHREAD_MUTEX_INITIALIZER;

#define NFF_LOCK()   pthread_mutex_lock(&nff_lock);
#define NFF_UNLOCK() pthread_mutex_unlock(&nff_lock);


static void dispatch_msg(char* data)
{
	int i;

	for (i = 0; i < NFF_BACKLOG; ++i) {
		NFF_LOCK();
		if (-1 != client_sock_list[i]) {
			D("send data to client %d", client_sock_list[i]);
			write(client_sock_list[i], data, strlen(data));
		}
		NFF_UNLOCK();
	}
}

static void* wait_on_fifo(void* arg)
{
	int fd, ret;
	char buf[NFF_BUF_SIZE] = {0};

	/* check if fifo file alread exist */
	if (0 == access(NFF_FIFO, F_OK)) {
		D("fifo file exist, delete it");
		if (-1 == remove(NFF_FIFO)) {
			E("delete fifo file failed");
			exit(-1);
		} else {
			E("delete fifo file success");
		}
	}

	if (-1 == mkfifo(NFF_FIFO, UGO_RW)) {
		E("create fifo failed");
		exit(-1);
	}

	if ((fd = open(NFF_FIFO, O_RDWR)) == -1) {
		E("open fifo failed");
		exit(-1);
	}

	for (;;) {
		memset(buf, 0, NFF_BUF_SIZE);
		ret = read(fd, buf, NFF_BUF_SIZE);
		if (ret > 0) {
			dispatch_msg(buf);
		}
	}

}

int main(int argc, char** argv)
{
	char buf[512] = {0};
	int listen_sock, conn_sock;
	struct sockaddr_in srv_addr, cli_addr;
	socklen_t addrlen;
	pid_t child_pid;
	fd_set orig_set, new_set;
	int maxfd, nready;
	int i, ret;

	for (i = 0; i < NFF_BACKLOG; ++i) {
		client_sock_list[i] = -1;
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listen_sock) {
		E("create socket failed");
		exit(-1);
	}

	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(NFF_DFT_PORT);

	ret = bind(listen_sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if (-1 == ret) {
		E("bind failed");
		exit(-1);
	}

	ret = listen(listen_sock, NFF_BACKLOG);
	if (-1 == ret) {
		E("listen failed");
		exit(-1);
	}

	// create a thread for wait on fifo
	pthread_create(&fifo_thread, NULL, wait_on_fifo, NULL);

	maxfd   = listen_sock;
	addrlen = sizeof(cli_addr);
	FD_ZERO(&orig_set);
	FD_ZERO(&new_set);
	FD_SET(listen_sock, &new_set);

	D("netfifo server running ...");

	for (;;) {
		orig_set = new_set;
		nready = select(maxfd + 1, &orig_set, NULL, NULL, NULL);

		if (FD_ISSET(listen_sock, &orig_set)) { /* new client connection */
			D("new client connect to server");
			conn_sock = accept(listen_sock, (struct sockaddr*)&cli_addr, &addrlen);

			if (-1 == conn_sock) {
				E("accept failed");
				continue;
			} else {
				for (i = 0; i < NFF_BACKLOG; ++i) { /* find a place for new client */
					if (-1 == client_sock_list[i]) {
						client_sock_list[i] = conn_sock;
						FD_SET(conn_sock, &new_set);
						D("add fd:%d to set", conn_sock);
						if (conn_sock > maxfd) {
							maxfd = conn_sock;
						}
						break;
					}
				}

				/* server cannot handle more clients, give a reply */
				if (i == NFF_BACKLOG) {
					D("server full");
					//+TODO: better reply msg
					NFF_LOCK();
					write(conn_sock, "server full", 12);
					close(conn_sock);
					NFF_UNLOCK();
				}

			}

			if ((--nready) <= 0) {
				continue;
			}
		}

		for (i = 0; i < NFF_BACKLOG; ++i) { /* check each client */
			if (FD_ISSET(client_sock_list[i], &orig_set)) {
				memset(buf, 0, NFF_BUF_SIZE);

				//+TODO: client => server protocol design
				NFF_LOCK();
				if ((ret = read(client_sock_list[i], buf, NFF_BUF_SIZE) == 0)) {
					D("client close connection");
					close(client_sock_list[i]);
					FD_CLR(client_sock_list[i], &new_set);
					client_sock_list[i] = -1;
				} else {
					D("recv: %s", buf);
				}
				NFF_UNLOCK();

				if ((--nready) <= 0) {
					continue;
				}
			}
		}
	}

	pthread_join(fifo_thread, NULL);
	close(listen_sock);
	return 0;
}
