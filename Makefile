CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
LDFLAGS = -L.

all : server client

server : server.c
	@echo ""
	@echo "***** Compilo server"
	$(CC) $(CFLAGS) -pthread server.c $(LDFLAGS) -o server

libs : libclient.c
	@echo ""
	@echo "***** Compilo librerie"
	$(CC) $(CFLAGS) libclient.c -c -o libclient.o
	ar rvs libclient.a libclient.o

client : libs client.c
	@echo ""
	@echo "***** Compilo client"
	$(CC) $(CFLAGS) client.c $(LDFLAGS) -lclient -o client

clean :
	rm -f client server libclient.a libclient.o objstore.sock testout.log *.log
	rm -rf data/
	./killserver.sh

test :
	@./scripttest.sh
	@echo ""
	@./testsum.sh

bigtest :
	@./bigtest.sh

dupobjtest :
	@./scripttest_oggettiduplicati.sh

dupclntest :
	@./scripttest_utentiduplicati.sh