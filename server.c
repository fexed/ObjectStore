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

#define UNIX_PATH_MAX 108
#define CHECK_RETVAL(value, str) \
		if (value != 0) { \
				perror(str); \
		}

int clientConnessi, oggettiMemorizzati, storeTotalSize;

struct sockaddr_un skta;
struct sigaction s;
int skt;

static void gest_SIGUSR1(int signum) {
	//TODO: info server in output
}

void cleanupserver() {
	close(skt);
	unlink(SOCKETNAME);
}

int startupserver() {
	int retval;
	memset(&s, 0, sizeof(s));
	s.sa_handler = gest_SIGUSR1;
	retval = sigaction(SIGUSR1, &s, NULL);
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

int main (int argc, char *argv[]) {
	int retval;

	retval = startupserver();
	CHECK_RETVAL(retval, "Server setup");

	return 0;
}