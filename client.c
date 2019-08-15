#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char *payload = "h9FqC9lYCgmACorQdu0IImWS8QPkdwvMAyjvNr1Sia0w0XSAjRn7NkJBrF5sh5s9SVtW59TFzwzVMzmw0RsveWhMmrRO3kPqBDD8";

int successi, fallimenti, testtotali;

void report(char *type) {
	printf("Test:%s\n%s DO:%d\n%s OK:%d\n%s KO:%d\n", type, type, testtotali, type, successi, type, fallimenti);
}

void testSTORE() {
	int i, j, n, len;
	char *substr, *name;

	len = 100;
	for (i = 0; i < 20; i++) {
		testtotali++;
		substr = malloc(len);
		memset(substr, 0, len);
		substr = strcpy(substr, payload);
		for (n = 0; n < i; n++) {
			for (j = 0; j < 52; j++) substr = strcat(substr, payload);
		}

		name = malloc(sizeof(len)+4);
		char strvalue[10];
		sprintf(strvalue, "%d", (int)len);
		name = strcpy(name, strvalue);
		name = strcat(name, "Byte");

		n = os_store(name, substr, strlen(substr));

		if (n == 0) successi++;
		else fallimenti++;

		len += 5200;
		free(substr);
		free(name);
	}

	os_disconnect();
	report("STORE");
}

void testRETRIEVE() {
	int i, j, n, len;
	char *substr, *name;
	char *data;

	len = 100;
	for (i = 0; i < 20; i++) {
		testtotali++;
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

		data = (char*)os_retrieve(name);
		if (data != NULL) n = strcmp((char*)substr, data);
		else n = -1;

		if (n == 0) successi++;
		else fallimenti++;

		len += 5200;
		free(substr);
		free(name);
	}

	os_disconnect();
	report("RETRIEVE");
}

void testDELETE() {
	int i, j, n, len;
	char *substr, *name;

	len = 100;
	for (i = 0; i < 20; i++) {
		testtotali++;
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

		if (n == 0) successi++;
		else fallimenti++;

		len += 5200;
		free(substr);
		free(name);
	}

	os_disconnect();
	report("REMOVE");
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: %s name n\n", argv[0]);
		return -1;
	} else {
		int tentativi, n;
		testtotali = 0;
		successi = 0;
		fallimenti = 0;

		for (tentativi = 0; tentativi < 5; tentativi++) {
			n = os_connect(argv[1]);


			if (n == 0) {
				printf("%s connesso\n", argv[1]);
				n = atoi(argv[2]);
				if (n == 1) testSTORE();
				else if (n == 2) testRETRIEVE();
				else if (n == 3) testDELETE();
				else printf("n deve essere tra 1 e 3 compresi\n");
				return 0;
			} else {
				sleep(1);
			} 
		}
		printf("Erorre %s: impossibile connettersi dopo 5 tentatitivi\n", argv[1]);
		return -2;
	}
}