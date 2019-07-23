#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
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
int skt, sktAccepted;
struct sockaddr_un skta;
struct sigaction s;
pthread_t threadpool[MAXTHREADS];
static pthread_mutex_t clientConnessiMutex = PTHREAD_MUTEX_INITIALIZER;

void cleanupserver() {
	close(skt);
	close(sktAccepted);
	unlink(SOCKETNAME);
}

static void signalHandler(int signum) {
	if (signum == SIGUSR1) {
		//TODO fix
	} else if (signum == SIGPIPE) {
	} else {
		exit(signum);
	}
}

int startupserver() {
	int retval;
	memset(&s, 0, sizeof(s));
	s.sa_handler = signalHandler;
	retval = sigaction(SIGUSR1, &s, NULL);
	retval = sigaction(SIGINT, &s, NULL);
	retval = sigaction(SIGPIPE, &s, NULL);
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

void incrementaClientConnessi() {
	pthread_mutex_lock(&clientConnessiMutex);
	clientConnessi++;
	pthread_mutex_unlock(&clientConnessiMutex);
}

void decrementaClientConnessi() {
	pthread_mutex_lock(&clientConnessiMutex);
	clientConnessi--;
	pthread_mutex_unlock(&clientConnessiMutex);
}

static void* clientHandler(void *arg) {
	int clientskt = (int) arg;
	int value;
	char *buff, *name, *header, *dirname;

	buff = calloc(BUFFSIZE, sizeof(char));
	read(clientskt, buff, BUFFSIZE);
	strtok(buff, " ");
	dirname = strtok(NULL, " ");
	name = malloc(sizeof(dirname));
	strcpy(name, dirname);
	printf("%s\tConnesso\n", name);
	
	//strcpy(dirname, name);
	value = mkdir(name, 0700);
	if (value != 0 && errno != EEXIST) {
		free(buff);
		buff = calloc(BUFFSIZE, sizeof(char));
		buff = strcpy(buff, "KO Errore creazione directory: ");
		char strvalue[10];
		sprintf(strvalue, "%d", value);
		buff = strcat(buff, strvalue);
		buff = strcat(buff, " \n");
		write(clientskt, buff, strlen(buff)+1);
		pthread_exit(NULL);
	}

	write(clientskt, "OK \n", 5);
	incrementaClientConnessi();

	do {
		read(clientskt, buff, BUFFSIZE);
		header = strtok(buff, " ");
		printf("%s\t%s\n", name, header);
		if (strcmp(header, "LEAVE")) {
			write(clientskt, "OK \n", 5);
			break;
		} else if (strcmp(header, "STORE")) {
			char *dataname, *datavalue;
			size_t datalen;

			dataname = strtok(NULL, " ");
			datalen = atoi(strtok(NULL, " "));
			strtok(NULL, " ");
			strtok(NULL, " ");
			datavalue = strtok(NULL, " ");

			printf("%s: %s, %d:\t%s\n", name, dataname, (int)datalen, datavalue);
			write(clientskt, "OK \n", 5);
		} else {
			printf("%s\t%s\n", name, header);
			write(clientskt, "OK \n", 5);
		}
	} while(1); //TODO fix

	close(clientskt);
	pthread_exit(NULL);
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
			retval = pthread_create(&threadpool[i], NULL, *clientHandler, (void *)sktAccepted);
			break;
		}
	} while(1); //TODO fix

	return 0;
}