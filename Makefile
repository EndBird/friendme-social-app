
PORT=52701
CFLAGS= -DPORT=\$(PORT) -g -Wall -D_POSIX_C_SOURCE

all: friends_server clean

friends_server: friendme.o friends.o 
	gcc $(CFLAGS) -Werror -std=c99 -o friends_server friendme.o friends.o

friends_server.o: friendme.c friends.c
	 gcc $(CFLAGS) -Werror -std=c99 -c friendme.c
friendme.o: friendme.c friends.h
	gcc $(CFLAGS) -Werror -std=c99 -c friendme.c
friends.o: friends.c friends.h
	gcc $(CFLAGS) -Werror -std=c99 -c friends.c

clean: 
	rm *.o