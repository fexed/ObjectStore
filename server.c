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
		}

volatile sig_atomic_t clientConnessi, oggettiMemorizzati, storeTotalSize, threads;
int skt, sktAccepted;
struct sockaddr_un skta;
struct sigaction s;
pthread_t threadpool[MAXTHREADS];
static pthread_mutex_t clientConnessiMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t threadAttiviMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t oggettiMemorizzatiMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t storeTotalSizeMutex = PTHREAD_MUTEX_INITIALIZER;

void cleanupserver() {
	close(skt);
	close(sktAccepted);
	unlink(SOCKETNAME);
}

static void signalHandler(int signum) {
	if (signum == SIGUSR1) {
		printf("threadAttivi\t\t%d\nclientConnessi\t\t%d\noggettiMemorizzati\t%d\nstoreTotalSize\t\t%d Byte\n", threads, clientConnessi, oggettiMemorizzati, storeTotalSize);
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

void incrementaOggettiMemorizzati() {
	pthread_mutex_lock(&oggettiMemorizzatiMutex);
	oggettiMemorizzati++;
	pthread_mutex_unlock(&oggettiMemorizzatiMutex);
}

void decrementaOggettiMemorizzati() {
	pthread_mutex_lock(&oggettiMemorizzatiMutex);
	oggettiMemorizzati++;
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

void mask_sign() {
	sigset_t mask;
	sigfillset(&mask);
	//sigaddset(&mask, SIGUSR1);
	pthread_sigmask(SIG_SETMASK, &mask, NULL);

}

static void* clientHandler(void *arg) {
	int clientskt = (int) arg;
	int value;
	char *buff, *name, *header, *dirname;
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
	read(clientskt, buff, BUFFSIZE);
	header = strtok(buff, "\n");

	if (header != NULL && strstr(header, "REGISTER") != NULL) {
		strtok(buff, " ");
		name = strcpy(name, strtok(NULL, " "));
		dirname = malloc(sizeof(name)+sizeof("data/"));
		dirname = strcpy(dirname, "data/");
		dirname = strcat(dirname, name);
		//printf("%s\tConnesso\n", name);

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
			write(clientskt, buff, BUFFSIZE);
			free(dirname);
			decrementaThreadAttivi();
			pthread_exit(NULL);
		}

		write(clientskt, "OK \n", BUFFSIZE);
		incrementaClientConnessi();

		do {
			free(buff);
			buff = calloc(BUFFSIZE, sizeof(char));
			buff = memset(buff, 0, BUFFSIZE);
			recv(clientskt, buff, BUFFSIZE, MSG_WAITALL);
			
			//printf("%s:\t%s", name, buff);

			header = strtok(buff, "\n");
			if (strstr(header, "STORE") != NULL) {
				strtok(buff, " ");
				dataname = strtok(NULL, " ");
				datalen = atoi(strtok(NULL, " "));
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
					free(buff);
					free(datavalue);
					buff = calloc(BUFFSIZE, sizeof(char));
					memset(buff, 0, BUFFSIZE);
					buff = strcpy(buff, "KO Errore: ");
					buff = strcat(buff, strerror(errno));
					buff = strcat(buff, " \n");
					write(clientskt, buff, BUFFSIZE);
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
			} else if (strstr(header, "RETRIEVE") != NULL) {
				strtok(buff, " ");
				dataname = strtok(NULL, " ");

				filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
				filename = strcpy(filename, dirname);
				filename = strcat(filename, "/");
				filename = strcat(filename, dataname);
				file = fopen(filename, "r");

				if (file == NULL) {
					free(filename);
					free(buff);
					buff = calloc(BUFFSIZE, sizeof(char));
					memset(buff, 0, BUFFSIZE);
					buff = strcpy(buff, "KO Errore: ");
					buff = strcat(buff, strerror(errno));
					buff = strcat(buff, " \n");
					write(clientskt, buff, BUFFSIZE);
				} else {
					//datalenr = calloc(1, sizeof(size_t));
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
			} else if (strstr(header, "DELETE") != NULL) {
				strtok(buff, " ");
				dataname = strtok(NULL, " ");

				filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
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
					free(buff);
					buff = calloc(BUFFSIZE, sizeof(char));
					memset(buff, 0, BUFFSIZE);
					buff = strcpy(buff, "KO Errore: ");
					buff = strcat(buff, strerror(errno));
					buff = strcat(buff, " \n");
					write(clientskt, buff, strlen(buff)+1);
				}
			} else if (strstr(header, "LEAVE") != NULL) {
				write(clientskt, "OK \n", BUFFSIZE);
				//printf("%s\tDisconnesso\n", name);
				free(dirname);
				free(name);
				close(clientskt);
				free(buff);
				decrementaClientConnessi();
				decrementaThreadAttivi();
				pthread_exit(NULL);
			} else {
				//printf("%s\t%s\n", name, buff);
				write(clientskt, "KO Comando non riconosciuto \n", BUFFSIZE);
			}
		} while(1); //TODO fix
		free(buff);
		close(clientskt);
		decrementaClientConnessi();
		decrementaThreadAttivi();
		pthread_exit(NULL);
	} else {
		free(buff);
		close(clientskt);
		decrementaThreadAttivi();
		pthread_exit(NULL);
	}
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
		}
	} while(1); //TODO fix

	return 0;
}