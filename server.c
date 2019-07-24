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
int threads;
static pthread_mutex_t clientConnessiMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t threadAttiviMutex = PTHREAD_MUTEX_INITIALIZER;

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
	int retval, i;
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

	mkdir("data", 0700);
	atexit(cleanupserver);

	clientConnessi = 0;
	threads = 0;
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

void incrementaThreadAttivi() {
	pthread_mutex_lock(&threadAttiviMutex);
	threads++;
	pthread_mutex_unlock(&threadAttiviMutex);
}

void decrementaThreadAttivi() {
	pthread_mutex_lock(&threadAttiviMutex);
	threads--;
	pthread_mutex_unlock(&threadAttiviMutex);
}

static void* clientHandler(void *arg) {
	int clientskt = (int) arg;
	int value;
	char *buff, *name, *header, *dirname;
	char *dataname, *datavalue, *filename;
	size_t datalen;
	FILE* file;

	incrementaThreadAttivi();
	buff = calloc(BUFFSIZE, sizeof(char));
	read(clientskt, buff, BUFFSIZE);
	header = strtok(buff, " ");
	//TODO if header = REGISTER...
	name = strtok(NULL, " ");
	dirname = malloc(sizeof(name)+sizeof("data/"));
	dirname = strcpy(dirname, "data/");
	dirname = strcat(dirname, name);
	printf("%s\tConnesso\n", name);

	value = mkdir(dirname, 0700);
	if (value != 0 && errno != EEXIST) {
		free(buff);
		buff = calloc(BUFFSIZE, sizeof(char));
		buff = strcpy(buff, "KO Errore: ");
		buff = strcat(buff, strerror(errno));
		buff = strcat(buff, " \n");
		char strvalue[10];
		sprintf(strvalue, "%d", value);
		buff = strcat(buff, strvalue);
		buff = strcat(buff, " \n");
		write(clientskt, buff, strlen(buff)+1);
		decrementaThreadAttivi();
		pthread_exit(NULL);
	}

	write(clientskt, "OK \n", 5);
	incrementaClientConnessi();

	do {
		free(buff);
		buff = calloc(BUFFSIZE, sizeof(char));
		read(clientskt, buff, BUFFSIZE);

		header = strtok(buff, " ");
		if (strcmp(header, "STORE") == 0) {
			dataname = strtok(NULL, " ");
			datalen = atoi(strtok(NULL, " "));
			strtok(NULL, " ");
			datavalue = strtok(NULL, " ");

			//printf("%s\t%s, %d:\t%s\n", name, dataname, (int)datalen, datavalue);

			filename = calloc(strlen(dirname)+strlen(dataname)+1, sizeof(char));
			filename = strcpy(filename, dirname);
			filename = strcat(filename, "/");
			filename = strcat(filename, dataname);
			file = fopen(filename, "w");

			if (file == NULL) {
				free(filename);
				free(buff);
				buff = calloc(BUFFSIZE, sizeof(char));
				buff = strcpy(buff, "KO Errore: ");
				buff = strcat(buff, strerror(errno));
				buff = strcat(buff, " \n");
				write(clientskt, buff, strlen(buff)+1);
			} else {
				fprintf(file, "%s", datavalue);
				fclose(file);
				free(filename);
				write(clientskt, "OK \n", 5);
			}
		} else if (strcmp(header, "RETRIEVE") == 0) {
			dataname = strtok(NULL, " ");

			filename = calloc(strlen(dirname)+strlen(dataname)+1, sizeof(char));
			filename = strcpy(filename, dirname);
			filename = strcat(filename, "/");
			filename = strcat(filename, dataname);
			file = fopen(filename, "r");

			if (file == NULL) {
				free(filename);
				free(buff);
				buff = calloc(BUFFSIZE, sizeof(char));
				buff = strcpy(buff, "KO Errore: ");
				buff = strcat(buff, strerror(errno));
				buff = strcat(buff, " \n");
				write(clientskt, buff, strlen(buff)+1);
			} else {
				datavalue = calloc(BUFFSIZE, sizeof(char));
				fgets(datavalue, BUFFSIZE, file);
				fclose(file);
				free(filename);

				free(buff);
				buff = calloc(BUFFSIZE, sizeof(char));
				buff = strcpy(buff, "DATA ");
				char strvalue[10];
				sprintf(strvalue, "%d", (int)strlen(datavalue));
				buff = strcat(buff, strvalue);
				buff = strcat(buff, " \n ");
				buff = strcat(buff, datavalue);

				write(clientskt, buff, strlen(buff));
			}
		} else if (strcmp(header, "DELETE") == 0) {
			dataname = strtok(NULL, " ");
			
			filename = calloc(strlen(dirname)+strlen(dataname)+1, sizeof(char));
			filename = strcpy(filename, dirname);
			filename = strcat(filename, "/");
			filename = strcat(filename, dataname);
			value = remove(filename);
			free(filename);

			if (value == 0) {
				write(clientskt, "OK \n", 5);
			} else {
				free(buff);
				buff = calloc(BUFFSIZE, sizeof(char));
				buff = strcpy(buff, "KO Errore: ");
				buff = strcat(buff, strerror(errno));
				buff = strcat(buff, " \n");
				write(clientskt, buff, strlen(buff)+1);
			}
		} else if (strcmp(header, "LEAVE") == 0) {
			write(clientskt, "OK \n", 5);
			printf("%s\tDisconnesso\n", name);

			close(clientskt);
			decrementaClientConnessi();
			decrementaThreadAttivi();
			pthread_exit(NULL);
		} else {
			printf("%s\t%s\n", name, header);
			write(clientskt, "OK \n", 5);
		}
	} while(1); //TODO fix

	close(clientskt);
	decrementaClientConnessi();
	decrementaThreadAttivi();
	pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
	int retval, i;

	retval = startupserver();
	CHECK_RETVAL(retval, "Server setup");

	do {
		listen(skt, SOMAXCONN);
		sktAccepted = accept(skt, NULL, 0);
		if (threads < MAXTHREADS) {
			retval = pthread_create(&threadpool[threads], NULL, *clientHandler, (void *)sktAccepted);
			//TODO fix                          ^^^^^^^
		}
	} while(1); //TODO fix

	return 0;
}