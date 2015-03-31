CFLAGS  += -Wall $(shell pkg-config --cflags json-c) -I../../base/include
LDFLAGS += $(shell pkg-config --libs json-c) $(shell pkg-config --libs libpcre) -lpthread
CC = gcc
RM = rm -fv

TARGET = logdog
OBJS   = main.o logdog_config.o logdog_action.o file_watcher.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) *.o
