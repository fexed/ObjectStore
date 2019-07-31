#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKETNAME "./objstore.sock"
#define MAXTHREADS 50
#define BUFFSIZE 100
#define UNIX_PATH_MAX 108
#define CHECK_RETVAL(value, str) \
		if (value != 0) { \
				perror(str); \
				return value; \
		}

volatile sig_atomic_t signaled;
int skt, sktAccepted;
struct sockaddr_un skta;
struct sigaction s;
pthread_t threadpool[MAXTHREADS];

int clientConnessi, oggettiMemorizzati, storeTotalSize, threads;
static pthread_mutex_t clientConnessiMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t threadAttiviMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t oggettiMemorizzatiMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t storeTotalSizeMutex = PTHREAD_MUTEX_INITIALIZER;

void incrementaOggettiMemorizzati() {
	pthread_mutex_lock(&oggettiMemorizzatiMutex);
	oggettiMemorizzati++;
	pthread_mutex_unlock(&oggettiMemorizzatiMutex);
}

void decrementaOggettiMemorizzati() {
	pthread_mutex_lock(&oggettiMemorizzatiMutex);
	oggettiMemorizzati--;
	pthread_mutex_unlock(&oggettiMemorizzatiMutex);
}

void incrementaStoreTotalSize(int size) {
	pthread_mutex_lock(&storeTotalSizeMutex);
	storeTotalSize += size;
	pthread_mutex_unlock(&storeTotalSizeMutex);
}

void decrementaStoreTotalSize(int size) {
	pthread_mutex_lock(&storeTotalSizeMutex);
	storeTotalSize -= size;
	pthread_mutex_unlock(&storeTotalSizeMutex);
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

void sendError(int clientskt, char *name, char *error) {
	char *buffer = calloc(BUFFSIZE, sizeof(char));
	memset(buffer, 0, BUFFSIZE);
	buffer = strcpy(buffer, "KO Errore ");
	buffer = strcat(buffer, name);
	buffer = strcat(buffer, ": ");
	buffer = strcat(buffer, error);
	buffer = strcat(buffer, " \n");
	write(clientskt, buffer, BUFFSIZE);
	free(buffer);
}

void mask_sign() {
	sigset_t mask;
	sigfillset(&mask);
	pthread_sigmask(SIG_SETMASK, &mask, NULL);

}

static void* clientHandler(void *arg) {
	int clientskt = (int) arg;
	int value, online = 0;
	char *buff, *savetoken, *name, *header, *dirname;
	char *dataname, *filename;
	void *datavalue;
	size_t datalen;
	FILE* file;

	mask_sign();
	incrementaThreadAttivi();
	name = calloc(BUFFSIZE, sizeof(char));
	name = memset(name, 0, BUFFSIZE);
	buff = calloc(BUFFSIZE, sizeof(char));
	buff = memset(buff, 0, BUFFSIZE);
	recv(clientskt, buff, BUFFSIZE, MSG_WAITALL);
	header = strtok_r(buff, "\n", &savetoken);

	if (header != NULL && strstr(header, "REGISTER") != NULL) {
		strtok_r(buff, " ", &savetoken);
		name = strcpy(name, strtok_r(NULL, " ", &savetoken));
		dirname = malloc(sizeof(name)+sizeof("data/"));
		dirname = strcpy(dirname, "data/");
		dirname = strcat(dirname, name);

		value = mkdir(dirname, 0700);
		if (value != 0 && errno != EEXIST) {
			sendError(clientskt, name, strerror(errno));
			free(dirname);
			decrementaThreadAttivi();
			pthread_exit(NULL);
		}

		write(clientskt, "OK \n", BUFFSIZE);
		online = 1;
		incrementaClientConnessi();

		do {
			free(buff);
			buff = calloc(BUFFSIZE, sizeof(char));
			buff = memset(buff, 0, BUFFSIZE);
			recv(clientskt, buff, BUFFSIZE, MSG_WAITALL);

			header = strtok_r(buff, " ", &savetoken);
			if (strcmp(header, "STORE") == 0) {
				dataname = strtok_r(NULL, " ", &savetoken);
				header = strtok_r(NULL, " ", &savetoken);
				datalen = strtol(header, (char **) NULL, 10);
				if (errno == EINVAL || errno == ERANGE) {
					buff = calloc(BUFFSIZE, sizeof(char));
					memset(buff, 0, BUFFSIZE);
					buff = strcpy(buff, "KO Errore: ");
					buff = strcat(buff, strerror(errno));
					buff = strcat(buff, " \n");
					write(clientskt, buff, BUFFSIZE);
					} else {
					datavalue = malloc(datalen);
					datavalue = memset(datavalue, 0, datalen);
					recv(clientskt, datavalue, datalen, MSG_WAITALL);

					filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
					filename = memset(filename, 0, sizeof(char)*(strlen(dirname)+strlen(dataname)+2));
					filename = strcpy(filename, dirname);
					filename = strcat(filename, "/");
					filename = strcat(filename, dataname);
					file = fopen(filename, "w");

					if (file == NULL) {
						free(filename);
						free(datavalue);
						sendError(clientskt, name, strerror(errno));
					} else {
						fwrite(&datalen, sizeof(size_t), 1, file);
						fwrite(datavalue, sizeof(char), datalen, file);
						fclose(file);
						free(filename);
						free(datavalue);
						write(clientskt, "OK \n", BUFFSIZE);
						incrementaStoreTotalSize((int) datalen);
						incrementaOggettiMemorizzati();
					}
				}
			} else if (strcmp(header, "RETRIEVE") == 0) {
				dataname = strtok_r(NULL, " ", &savetoken);

				filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
				filename = memset(filename, 0, sizeof(char)*(strlen(dirname)+strlen(dataname)+2));
				filename = strcpy(filename, dirname);
				filename = strcat(filename, "/");
				filename = strcat(filename, dataname);
				file = fopen(filename, "r");

				if (file == NULL) {
					free(filename);
					sendError(clientskt, name, strerror(errno));
				} else {
					fread(&datalen, sizeof(size_t), 1, file);
					datavalue = calloc(1, datalen);
					memset(datavalue, 0, datalen);
					fread(datavalue, datalen, 1, file);
					fclose(file);
					free(filename);

					free(buff);
					buff = calloc(BUFFSIZE, sizeof(char));
					memset(buff, 0, BUFFSIZE);
					buff = strcpy(buff, "DATA ");
					char strvalue[10];
					sprintf(strvalue, "%ld", datalen);
					buff = strcat(buff, strvalue);
					buff = strcat(buff, " \n");

					write(clientskt, buff, BUFFSIZE);
					write(clientskt, datavalue, datalen);
					free(datavalue);
				}
			} else if (strcmp(header, "DELETE") == 0) {
				dataname = strtok_r(NULL, " ", &savetoken);

				filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
				filename = memset(filename, 0, sizeof(char)*(strlen(dirname)+strlen(dataname)+2));
				filename = strcpy(filename, dirname);
				filename = strcat(filename, "/");
				filename = strcat(filename, dataname);

				file = fopen(filename, "r");
				if (file != NULL) {
					fread(&datalen, sizeof(size_t), 1, file);
					fclose(file);
				}

				value = remove(filename);
				free(filename);

				if (value == 0) {
					write(clientskt, "OK \n", BUFFSIZE);
					decrementaStoreTotalSize((int) datalen);
					decrementaOggettiMemorizzati();
				} else {
					sendError(clientskt, name, strerror(errno));
				}
			} else if (strstr(header, "LEAVE") != NULL) {
				write(clientskt, "OK \n", BUFFSIZE);

				free(dirname);
				free(name);
				online = 0;
			} else {
				sendError(clientskt, name, "Comando non riconosciuto");
			}
		} while(online == 1);

		free(buff);
		close(clientskt);
		decrementaClientConnessi();
		decrementaThreadAttivi();
		pthread_exit(NULL);
	} else {
		sendError(clientskt, name, "Comando non riconosciuto");

		free(buff);
		close(clientskt);

		decrementaThreadAttivi();
		pthread_exit(NULL);
	}
}

void cleanupserver() {
	close(skt);
	close(sktAccepted);
	unlink(SOCKETNAME);
}

static void signalHandler(int signum) {
	if (signum == SIGUSR1) {
		signaled = 1;
	} else if (signum == SIGPIPE) { //ignore
	} else {
		cleanupserver();
		exit(signum);
	}
}

int startupserver() {
	int retval;

	memset(&s, 0, sizeof(s));
	s.sa_handler = signalHandler;
	retval = sigaction(SIGUSR1, &s, NULL);
	if (retval != 0) return retval;
	retval = sigaction(SIGPIPE, &s, NULL);
	if (retval != 0) return retval;
	retval = sigaction(SIGINT, &s, NULL);
	if (retval != 0) return retval;
	retval = sigaction(SIGTERM, &s, NULL);
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
	signaled = 0;

	return retval;
}

int main () {
	int retval;

	retval = startupserver();
	CHECK_RETVAL(retval, "Server setup");

	do {
		listen(skt, SOMAXCONN);
		sktAccepted = accept(skt, NULL, 0);
		if (threads < MAXTHREADS) {
			retval = pthread_create(&threadpool[threads], NULL, *clientHandler, (void *)sktAccepted);
			//TODO fix                          ^^^^^^^
			if (retval != 0) sendError(sktAccepted, "<non connesso>", "Impossibile istanziare il thread");
		}

		if (signaled == 1) {
			signaled = 0;
			printf("Thread attivi:\t\t\t\t%d\nClient connessi:\t\t\t%d\nOggetti attualmente in store:\t\t%d\nDimensione totale store:\t\t%d Byte\n", threads, clientConnessi, oggettiMemorizzati, storeTotalSize);
		}
	} while(1); //TODO fix

	return 0;
}