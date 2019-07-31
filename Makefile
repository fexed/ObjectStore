CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
LDFLAGS = -L.

all :
	@echo "Compilo librerie"
	$(CC) $(CFLAGS) libclient.c -c -o libclient.o
	ar rvs libclient.a libclient.o
	@echo ""
	@echo "Compilo client"
	$(CC) $(CFLAGS) client.c $(LDFLAGS) -lclient -o client
	@echo ""
	@echo "Compilo server"
	$(CC) $(CFLAGS) -pthread server.c $(LDFLAGS) -o server

clean :
	rm -f client server libclient.a libclient.o objstore.sock testout.log
	rm -rf data/
	./killserver.sh

test :
	@./scripttest.sh
	@echo ""
	@./testsum.sh

bigtest :
	./bigtest.sh