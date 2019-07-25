#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char *payload = "h9FqC9lYCgmACorQdu0IImWS8QPkdwvMAyjvNr1Sia0w0XSAjRn7NkJBrF5sh5s9SVtW59TFzwzVMzmw0RsveWhMmrRO3kPqBDD8";

struct object {
	char* name;
	void* value;
	size_t len;
};

int testSTORE() {
	int i, j, n, len;
	char *substr, *name;
	printf("Test STORE\n");

	len = 100;
	for (i = 0; i < 20; i++) {
		substr = malloc(len);
		substr = strcpy(substr, payload);
		for (n = 0; n < i; n++) {
			for (j = 0; j < 52; j++) substr = strcat(substr, payload);
		}

		name = malloc(sizeof(len)+4);
		char strvalue[10];
		sprintf(strvalue, "%d", (int)len);
		name = strcpy(name, strvalue);
		name = strcat(name, "Byte");

		n = os_store(name, substr, len);

		if (n != 0) return n;

		len += 5200;
		free(substr);
		free(name);
	}

	os_disconnect();
	return n;
}

int testRETRIEVE() {
	int i, j, n, len;
	char *substr, *name;
	printf("Test RETRIEVE\n");

	len = 100;
	for (i = 0; i < 20; i++) {
		substr = malloc(len);
		substr = strcpy(substr, payload);
		for (n = 0; n < i; n++) {
			for (j = 0; j < 52; j++) substr = strcat(substr, payload);
		}

		name = malloc(sizeof(len)+4);
		char strvalue[10];
		sprintf(strvalue, "%d", (int)len);
		name = strcpy(name, strvalue);
		name = strcat(name, "Byte");

		n = strcmp((char*)substr, (char*)os_retrieve(name));

		if (n != 0) return n;

		len += 5200;
		free(substr);
		free(name);
	}

	os_disconnect();
	return n;
}

int testDELETE() {
	int i, j, n, len;
	char *substr, *name;
	printf("Test REMOVE\n");

	len = 100;
	for (i = 0; i < 20; i++) {
		substr = malloc(len);
		substr = strcpy(substr, payload);
		for (n = 0; n < i; n++) {
			for (j = 0; j < 52; j++) substr = strcat(substr, payload);
		}

		name = malloc(sizeof(len)+4);
		char strvalue[10];
		sprintf(strvalue, "%d", (int)len);
		name = strcpy(name, strvalue);
		name = strcat(name, "Byte");

		n = os_delete(name);

		if (n != 0) return n;

		len += 5200;
		free(substr);
		free(name);
	}
	
	os_disconnect();
	return n;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: %s name n\n", argv[0]);
		return -1;
	} else {
		int n = os_connect(argv[1]);

		if (n == 0) {
			n = atoi(argv[2]);
			if (n == 1) return testSTORE();
			else if (n == 2) return testRETRIEVE();
			else if (n == 3) return testDELETE();
			else printf("n deve essere tra 1 e 3 compresi\n");
		} else return -2;
	}
	return 0;
}