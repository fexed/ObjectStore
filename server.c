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
#define MAXTHREADS 100
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
pthread_t threadt;
char* clients[MAXTHREADS];

int clientConnessi, oggettiMemorizzati, storeTotalSize, threads;
static pthread_mutex_t clientConnessiMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t clientsCheck = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t threadAttiviMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t oggettiMemorizzatiMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t storeTotalSizeMutex = PTHREAD_MUTEX_INITIALIZER;

void registerClient(char* name, int freepos) {
	clients[freepos] = calloc(strlen(name)+1, sizeof(char));
	strcpy(clients[freepos], name);
}

int checkClient(char* name) {
	int i;
	int freepos = -1;
	for (i = 0; i < MAXTHREADS; i++) {
		if (clients[i] == NULL && freepos == -1) {
			freepos = i;
			registerClient(name, freepos);
			i = MAXTHREADS;
		} else if (clients[i] != NULL) if (strcmp(name, clients[i]) == 0) return -2;
	}
	return freepos; /*-1: pieno, -2: cliente già registrato*/
}

void deregisterClient(int freepos) {
	pthread_mutex_lock(&clientsCheck);
	free(clients[freepos]);
	clients[freepos] = NULL;
	pthread_mutex_unlock(&clientsCheck);
}

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
	if (buffer == NULL) {
		write(clientskt, "KO Errore sconosciuto e errore nella calloc \n", BUFFSIZE);
		} else {
		memset(buffer, 0, BUFFSIZE);
		buffer = strcpy(buffer, "KO Errore ");
		buffer = strcat(buffer, name);
		buffer = strcat(buffer, ": ");
		buffer = strcat(buffer, error);
		buffer = strcat(buffer, " \n");
		write(clientskt, buffer, BUFFSIZE);
		free(buffer);
	}
}

void mask_sign() {
	sigset_t mask;
	sigfillset(&mask);
	pthread_sigmask(SIG_SETMASK, &mask, NULL);

}

static void* clientHandler(void *arg) {
	int clientskt = (int) arg;
	int value, online = 0, freepos;
	char *buff, *savetoken, *name, *header, *dirname;
	char *dataname, *filename;
	void *datavalue;
	size_t datalen;
	struct stat filestat;
	FILE* file;
	char strvalue[10];

	mask_sign();
	pthread_detach(pthread_self());
	incrementaThreadAttivi();
	
	buff = calloc(BUFFSIZE, sizeof(char));
	if (buff == NULL) {
		sendError(clientskt, "<non iniz.>", "Inizializzazione del thread fallita");

		close(clientskt);
		decrementaThreadAttivi();
		return NULL; /*fix per un fake leak di pthread*/
	}

	recv(clientskt, buff, BUFFSIZE, MSG_WAITALL);
	header = strtok_r(buff, "\n", &savetoken);

	if (header != NULL && strstr(header, "REGISTER") != NULL) {
		strtok_r(buff, " ", &savetoken);
		name = calloc(BUFFSIZE, sizeof(char));
		if (name == NULL) {
			sendError(clientskt, "<non iniz.>", "Inizializzazione del thread fallita");

			free(buff);
			close(clientskt);
			decrementaThreadAttivi();
			return NULL; /*fix per un fake leak di pthread*/
		}

		name = strcpy(name, strtok_r(NULL, " ", &savetoken));

		pthread_mutex_lock(&clientsCheck);
		freepos = checkClient(name);
		pthread_mutex_unlock(&clientsCheck);
		if (freepos >= 0) {
			dirname = calloc(strlen(name)+strlen("data/")+1, sizeof(char));
			dirname = strcpy(dirname, "data/");
			dirname = strcat(dirname, name);

			value = mkdir(dirname, 0700);
			if (value != 0 && errno != EEXIST) {
				sendError(clientskt, name, strerror(errno));
				free(name);
				free(buff);
				free(header);
				free(dirname);
				decrementaThreadAttivi();
				return NULL; /*fix per un fake leak di pthread*/
			}

			write(clientskt, "OK \n", BUFFSIZE);
			online = 1;
			incrementaClientConnessi();

			do {
				free(buff);
				buff = calloc(BUFFSIZE, sizeof(char));
				if (buff == NULL) {
					sendError(clientskt, name, "Errore allocazione buffer");
					online = 1;
				} else {
					recv(clientskt, buff, BUFFSIZE, MSG_WAITALL);

					header = strtok_r(buff, " ", &savetoken);
					if (strcmp(header, "STORE") == 0) {
						dataname = strtok_r(NULL, " ", &savetoken);
						header = strtok_r(NULL, " ", &savetoken);
						datalen = strtol(header, (char **) NULL, 10);
						if (errno == EINVAL || errno == ERANGE) {
							sendError(clientskt, name, strerror(errno));
							} else {
							datavalue = calloc(datalen, sizeof(char));

							if (datavalue == NULL) {
								sendError(clientskt, name, "Errore allocazione buffer per i dati");
							} else {
								recv(clientskt, datavalue, datalen, MSG_WAITALL);

								filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
								if (filename == NULL) {
									sendError(clientskt, name, "Errore allocazione buffer");
								} else {
									filename = strcpy(filename, dirname);
									filename = strcat(filename, "/");
									filename = strcat(filename, dataname);
									value = stat(filename, &filestat);
									if (value == 0) {
										free(filename);
										free(datavalue);
										sendError(clientskt, name, "File già esistente");
									} else {

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
								}
							}
						}
					} else if (strcmp(header, "RETRIEVE") == 0) {
						dataname = strtok_r(NULL, " ", &savetoken);

						filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
						if (filename == NULL) {
							sendError(clientskt, name, "Errore allocazione buffer");
						} else {
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
								sprintf(strvalue, "%ld", datalen);
								buff = strcat(buff, strvalue);
								buff = strcat(buff, " \n");

								write(clientskt, buff, BUFFSIZE);
								write(clientskt, datavalue, datalen);
								free(datavalue);
							}
						}
					} else if (strcmp(header, "DELETE") == 0) {
						dataname = strtok_r(NULL, " ", &savetoken);

						filename = calloc(strlen(dirname)+strlen(dataname)+2, sizeof(char));
						if (filename == NULL) {
							sendError(clientskt, name, "Errore allocazione buffer");
						} else {
							filename = strcpy(filename, dirname);
							filename = strcat(filename, "/");
							filename = strcat(filename, dataname);

							file = fopen(filename, "r");
							if (file != NULL) {
								fread(&datalen, sizeof(size_t), 1, file);
								fclose(file);
								value = remove(filename);
								free(filename);

								if (value == 0) {
									write(clientskt, "OK \n", BUFFSIZE);
									decrementaStoreTotalSize((int) datalen);
									decrementaOggettiMemorizzati();
								} else {
									sendError(clientskt, name, strerror(errno));
								}
							} else {
								free(filename);
								sendError(clientskt, name, strerror(errno));
							}
						}
					} else if (strstr(header, "LEAVE") != NULL) {
						write(clientskt, "OK \n", BUFFSIZE);

						free(dirname);
						free(name);
						online = 0;
					} else {
						sendError(clientskt, name, "Comando non riconosciuto");
					}
				}
			} while(online == 1);

			free(buff);
			close(clientskt);
			deregisterClient(freepos);
			decrementaClientConnessi();
			decrementaThreadAttivi();
			return NULL; /*fix per un fake leak di pthread*/
		} else {
			if (freepos == -1) sendError(clientskt, name, "Impossibile istanziare altri thread"); /*non dovrebbe mai succedere*/
			else if (freepos == -2) sendError(clientskt, name, "Nome già registrato");

			free(name);
			free(buff);
			close(clientskt);
			decrementaThreadAttivi();
			return NULL; /*fix per un fake leak di pthread*/
			/*pthread_cancel_init (unwind-forcedunwind.c:52)*/
			/*https://stackoverflow.com/questions/9951891/c-cleanup-unused-threads*/
		}
	} else {
		sendError(clientskt, "<non iniz.>", "Comando non riconosciuto");

		free(buff);
		close(clientskt);
		decrementaThreadAttivi();
		return NULL; /*fix per un fake leak di pthread*/
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
	} else if (signum == SIGPIPE) {
	} else {
		cleanupserver();
		exit(signum);
	}
}

int startupserver() {
	int retval, i;
	struct sigaction s;

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

	for (i = 0; i < MAXTHREADS; i++) {
		free(clients[i]);
		clients[i] = NULL;
	}

	return retval;
}

int main () {
	int retval, online;

	retval = startupserver();
	CHECK_RETVAL(retval, "Server setup");

	online = 1;
	do {
		listen(skt, SOMAXCONN);
		sktAccepted = accept(skt, NULL, 0);
		if (threads < MAXTHREADS) {
			retval = pthread_create(&threadt, NULL, *clientHandler, (void *)sktAccepted);
			if (retval != 0) sendError(sktAccepted, "<non connesso>", "Impossibile istanziare il thread");
		} else {
			sendError(sktAccepted, "<non connesso>", "Troppi thread connessi");
		}

		if (signaled == 1) {
			signaled = 0;
			printf("Thread attivi:\t\t\t\t%d\nClient connessi:\t\t\t%d\nOggetti attualmente in store:\t\t%d\nDimensione totale store:\t\t%d Byte\n", threads, clientConnessi, oggettiMemorizzati, storeTotalSize);
			online = 0;
		}
	} while(online);

	return 0;
}