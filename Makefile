CFLAGS := -ggdb -Wall $(CFLAGS)

.SUFFIXES:
.SUFFIXES: .o .c

.PHONY: all
all: cont

cont: main.c net.o
	$(CC) $(CFLAGS) -o $@ $^

net.o: net/rt_rtnetlink.o net/nl_netlink.o net/net_network.o
	ld -r -o $@ $^

.PHONY: clean
clean:
	-rm -f net/*.o
	-rm -f *.o
	-rm -f cont
