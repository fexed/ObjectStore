CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
LDFLAGS = -L.

all :
	@echo "Compiling"
	$(CC) $(CFLAGS) libclient.c -c -o libclient.o
	ar rvs libclient.a libclient.o
	$(CC) $(CFLAGS) client.c $(LDFLAGS) -lclient -o client
	$(CC) $(CFLAGS) -pthread server.c $(LDFLAGS) -o server

clean :
	rm -f client server libclient.a libclient.o objstore.sock testout.log
	rm -rf data/

test :
	./scripttest.sh