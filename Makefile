CC = gcc
CFLAGS = -Wall -pedantic -g
LDFLAGS = -L.

all :
	@echo "Compiling"
	$(CC) $(CFLAGS) libclient.c -c -o libclient.o
	ar rvs libclient.a libclient.o
	$(CC) $(CFLAGS) client.c $(LDFLAGS) -lclient -o client
	$(CC) $(CFLAGS) server.c $(LDFLAGS) -o server

clean :
	rm -f filetolist libList.a libList.o