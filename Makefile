CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu99

all: transport

transport: tr_helpers.o transport.o
	$(CC) transport.o tr_helpers.o -o transport $(CFLAGS)

clean:
	rm -f *.o

distclean:
	rm -f transport *.o
