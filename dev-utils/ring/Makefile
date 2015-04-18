INCLUDE_PATH = -I../../base/include

CFLAGS += $(INCLUDE_PATH)
OBJS += ring.o

ring: $(OBJS)
	$(CC) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	@find . -name "*.o" | xargs rm -fv
