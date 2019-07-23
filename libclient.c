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
		if (errno == ENOENT) sleep(1); //socket non esiste
		else return -1;
	}

	buff = calloc(BUFFSIZE, sizeof(char));
	buff = strcpy(buff, "REGISTER ");
	buff = strcat(buff, name);
	buff = strcat(buff, " \n");
	write(skt, buff, BUFFSIZE);
	
	read(skt, buff, BUFFSIZE);

	value = strcmp(buff, "OK \n");

	return value;
}

int os_store(char *name, void *block, size_t len) {
	char *buff = calloc(BUFFSIZE, sizeof(char));
	buff = strcpy(buff, "STORE ");
	buff = strcat(buff, name);
	buff = strcat(buff, " ");
	buff = strcat(buff, (int) len); //fix?
	buff = strcat(buff, " \n ");
	buff = strcat(buff, block);
	write(skt, buff, BUFFSIZE);
}

void *os_retrieve(char *name);

int os_delete(char *name);

int os_disconnect() {
	write(skt, "LEAVE \n", 8);
	close(skt);
	return 0;
}