#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKETNAME "./objectstore_530257"
#define MAXTHREADS 25
#define BUFFSIZE 100
#define UNIX_PATH_MAX 108
#define CHECK_RETVAL(value, str) \
		if (value != 0) { \
				perror(str); \
		}

int clientConnessi, oggettiMemorizzati, storeTotalSize;
pthread_t threadpool[MAXTHREADS];
struct sockaddr_un skta;
struct sigaction s;
int skt, sktAccepted;

void cleanupserver() {
	close(skt);
	close(sktAccepted);
	unlink(SOCKETNAME);
}

static void signalHandler(int signum) {
	if (signum == SIGUSR1) {
		//TODO: info server in output
	} else {
		//cleanupserver();
		exit(EXIT_SUCCESS);
	}
}

int startupserver() {
	int retval;
	memset(&s, 0, sizeof(s));
	s.sa_handler = signalHandler;
	retval = sigaction(SIGUSR1, &s, NULL);
	retval = sigaction(SIGINT, &s, NULL);
	if (retval != 0) return retval;
	
	strncpy(skta.sun_path, SOCKETNAME, UNIX_PATH_MAX);
	skta.sun_family = AF_UNIX;
	skt = socket(AF_UNIX, SOCK_STREAM, 0);
	retval = bind(skt, (struct sockaddr *)&skta, sizeof(skta));
	if (retval != 0) return retval;

	atexit(cleanupserver);

	clientConnessi = 0;
	oggettiMemorizzati = 0;
	storeTotalSize = 0;

	return retval;
}

static void* clientHandler(void *arg) {
	int clientskt = (int) arg;
	char buff[BUFFSIZE];

	read(clientskt, buff, BUFFSIZE);
	printf("%s", buff);

	close(clientskt);
}

int main (int argc, char *argv[]) {
	int retval, i;

	retval = startupserver();
	CHECK_RETVAL(retval, "Server setup");

	do {
		listen(skt, SOMAXCONN);
		sktAccepted = accept(skt, NULL, 0);
		for (i = 0; i < MAXTHREADS; i++) {
			//TODO check thread vuoto
				retval = pthread_create(&threadpool[i], NULL, *clientHandler, sktAccepted);
				break;
		}
	} while(1);

	return 0;
}