# Makefile for netpipe

TARGET  = nffd nffc
SRV_OBJ = netfifo_server.o
CLI_OBJ = netfifo_client.o
CC      = gcc
RM      = rm -rf
CFLAGS	 +=

all: $(TARGET)

nffd: $(SRV_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

nffc: $(CLI_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(TARGET) *.o
