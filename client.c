#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char *payload = "h9FqC9lYCgmACorQdu0IImWS8QPkdwvMAyjvNr1Sia0w0XSAjRn7NkJBrF5sh5s9SVtW59TFzwzVMzmw0RsveWhMmrRO3kPqBDD8";

int successi, fallimenti, testtotali;

void report(char *type) {
	printf("Test %s\n%s effettuati\t%d\n%s successi\t%d\n%s fallimenti\t%d\n", type, type, testtotali, type, successi, type, fallimenti);
}

void testSTORE() {
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

		//printf("Spedisco %s, %d:\t%.15s...\n", name, len, substr);
		n = os_store(name, substr, strlen(substr));

		if (n == 0) successi++;
		else /*fallimenti++;*/exit(n);

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

		n = strcmp((char*)substr, (char*)os_retrieve(name));

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
		testtotali = 0;
		successi = 0;
		fallimenti = 0;
		int n = os_connect(argv[1]);

		if (n == 0) {
			n = atoi(argv[2]);
			if (n == 1) testSTORE();
			else if (n == 2) testRETRIEVE();
			else if (n == 3) testDELETE();
			else printf("n deve essere tra 1 e 3 compresi\n");
			return 0;
		} else return -2;
	}
	return 0;
}