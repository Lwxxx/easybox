VPATH = core common component
INCLUDE_PATH = -I./component -I./core -I./common -I../../../base/include

CFLAGS += $(INCLUDE_PATH)
LDLIBS += -lpthread
OBJS += ddd.o ddd_socket.o portmap.o command.o tftp.o log.o monitor.o

ddd: $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	@find . -name "*.o" | xargs rm -fv
