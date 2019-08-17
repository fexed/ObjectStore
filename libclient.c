#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int skt;

int os_connect(char *name){
	struct sockaddr_un skta;
	char *buff;
	int value;

	strncpy(skta.sun_path, SOCKETNAME, UNIX_PATH_MAX);
	skta.sun_family = AF_UNIX;
	skt = socket(AF_UNIX, SOCK_STREAM, 0);
	while(connect(skt, (struct sockaddr*)&skta, sizeof(skta)) == -1) {
		if (errno == ENOENT) sleep(1);
		else return -1;
	}

	buff = calloc(BUFFSIZE, sizeof(char));
	if (buff == NULL) {
		printf("Errore: generazione del messaggio di registrazione fallita\n");
		return -1;
	}
	buff = memset(buff, 0, BUFFSIZE);
	buff = strcpy(buff, "REGISTER ");
	buff = strcat(buff, name);
	buff = strcat(buff, " \n");
	write(skt, buff, BUFFSIZE);
	
	read(skt, buff, BUFFSIZE);

	value = strcmp(buff, "OK \n");

	if (value != 0) {
		strtok(buff, " ");
		printf("%s\n", strtok(NULL, "\n"));
	}

	free(buff);
	return value;
}

int os_store(char *name, void *block, size_t len) {
	char *buff = calloc(BUFFSIZE, sizeof(char));
	int value;
	char strvalue[10];

	if (buff == NULL) {
		printf("Errore: generazione del messaggio di memorizzazione di %s fallita\n", name);
		return -1;
	}
	buff = memset(buff, 0, BUFFSIZE);
	buff = strcpy(buff, "STORE ");
	buff = strcat(buff, name);
	buff = strcat(buff, " ");
	sprintf(strvalue, "%ld", len);
	buff = strcat(buff, strvalue);
	buff = strcat(buff, " \n");

	write(skt, buff, BUFFSIZE);
	
	free(buff);
	buff = calloc(BUFFSIZE, sizeof(char));
	buff = memset(buff, 0, BUFFSIZE);

	write(skt, block, len);

	read(skt, buff, BUFFSIZE);
	value = strcmp(buff, "OK \n");

	if (value != 0) {
		strtok(buff, " ");
		printf("%s\n", strtok(NULL, "\n"));
	}

	free(buff);
	return value;
}

void *os_retrieve(char *name) {
	char *buff = calloc(BUFFSIZE, sizeof(char));
	char *header;
	void *datavalue;
	size_t len;

	if (buff == NULL) {
		printf("Errore: generazione del messaggio di ottenimento di %s fallita\n", name);
		return NULL;
	}
	buff = strcpy(buff, "RETRIEVE ");
	buff = strcat(buff, name);
	buff = strcat(buff, " \n");

	write(skt, buff, BUFFSIZE);

	recv(skt, buff, BUFFSIZE, MSG_WAITALL);
	header = strtok(buff, " ");

	if(strcmp(header, "DATA") == 0) {
		len = atoi(strtok(NULL, " "));
		datavalue = calloc(len, sizeof(char));

		recv(skt, datavalue, len, MSG_WAITALL);
		return datavalue;
	} else {
		printf("%s\n", strtok(NULL, "\n"));
		return NULL;
	}
}

int os_delete(char *name) {
	char *buff = calloc(BUFFSIZE, sizeof(char));
	int value;

	if (buff == NULL) {
		printf("Errore: generazione del messaggio di rimozione di %s fallita\n", name);
		return -1;
	}
	buff = strcpy(buff, "DELETE ");
	buff = strcat(buff, name);
	buff = strcat(buff, " \n");

	write(skt, buff, BUFFSIZE);

	read(skt, buff, BUFFSIZE);

	value = strcmp(buff, "OK \n");

	if (value != 0) {
		strtok(buff, " ");
		printf("%s\n", strtok(NULL, "\n"));
	}

	return value;
}

int os_disconnect() {
	write(skt, "LEAVE \n", BUFFSIZE);
	close(skt);
	return 0;
}