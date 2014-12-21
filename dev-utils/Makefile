TARGET = cscope-indexer
CFLAGS = -Wall
CC = gcc
RM = rm -fv

all: $(TARGET)

cscope-indexer: cscope-indexer.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) *.o $(TARGET)
