CFLAGS := -ggdb -Werror -Wall -std=c11 -pedantic $(CFLAGS)

.SUFFIXES:
.SUFFIXES: .o .c

.PHONY: all
all: cont

cont: main.c net.o
	$(CC) $(CFLAGS) -o $@ $^

test: test.c net_test.o
	$(CC) $(CFLAGS) -o $@ $^ `pkg-config --cflags --libs check`

net_test.o: net.o net/net_test.o
	ld -r -o $@ $^

net.o: net/rt_rtnetlink.o net/nl_netlink.o net/net_network.o
	ld -r -o $@ $^

.PHONY: clean
clean:
	-rm -f net/*.o
	-rm -f *.o
	-rm -f cont
