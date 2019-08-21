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
	char strvalue[10];

	len = 100;
	substr = malloc((len+1)*sizeof(char));
	for (i = 0; i < 20; i++) {
		testtotali++;
		if (substr == NULL) {
			printf("Errore allocazione substr");
			fallimenti++;
		} else {
			if (i > 0) {
				for (j = 0; j < 53; j++) {
					if (j < 52) substr = strcat(substr, payload);
					else substr = strncat(substr, payload, 58);
				}
			} else {
				substr = strcpy(substr, payload);
			}
				sprintf(strvalue, "%d", (int)len);
				name = calloc(strlen(strvalue)+5, sizeof(char));
				if (name == NULL) {
					printf("Errore allocazione nome");
					fallimenti++;
				} else {
				name = strcpy(name, strvalue);
				name = strcat(name, "Byte");
				n = os_store(name, substr, strlen(substr));

				if (n == 0) successi++;
				else fallimenti++;

				len += 5258;
				free(name);
				substr = realloc(substr, (len+1)*sizeof(char));
			}
		}
	}

	free(substr);
	os_disconnect();
	report("STORE");
}

void testRETRIEVE() {
	int i, j, n, len;
	char *substr, *name;
	char *data;
	char strvalue[10];

	len = 100;
	substr = malloc((len+1)*sizeof(char));
	for (i = 0; i < 20; i++) {
		testtotali++;
		if (substr == NULL) {
			printf("Errore allocazione substr");
			fallimenti++;
		} else {
			if (i > 0) {
				for (j = 0; j < 53; j++) {
					if (j < 52) substr = strcat(substr, payload);
					else substr = strncat(substr, payload, 58);
				}
			} else {
				substr = strcpy(substr, payload);
			}
				sprintf(strvalue, "%d", (int)len);
				name = calloc(strlen(strvalue)+5, sizeof(char));
				if (name == NULL) {
					printf("Errore allocazione nome");
					fallimenti++;
				} else {
				name = strcpy(name, strvalue);
				name = strcat(name, "Byte");

				data = (char*)os_retrieve(name);
				if (data != NULL) n = strcmp((char*)substr, data);
				else n = -1;

				if (n == 0) successi++;
				else fallimenti++;

				len += 5258;
				free(name);
				free(data);
				substr = realloc(substr, (len+1)*sizeof(char));
			}
		}
	}

	free(substr);
	os_disconnect();
	report("RETRIEVE");
}

void testDELETE() {
	int i, n, len;
	char *name;
	char strvalue[10];

	len = 100;
	for (i = 0; i < 20; i++) {
		testtotali++;

			sprintf(strvalue, "%d", (int)len);
			name = calloc(strlen(strvalue)+5, sizeof(char));
			if (name == NULL) {
				printf("Errore allocazione nome");
				fallimenti++;
			} else {
			name = strcpy(name, strvalue);
			name = strcat(name, "Byte");

			n = os_delete(name);

			if (n == 0) successi++;
			else fallimenti++;

			len += 5258;
			free(name);
		}
	}

	os_disconnect();
	report("REMOVE");
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: %s name n\n", argv[0]);
		return -1;
	} else {
		int n;
		testtotali = 0;
		successi = 0;
		fallimenti = 0;

		n = os_connect(argv[1]);


		if (n == 0) {
			n = atoi(argv[2]);
			if (n == 1) testSTORE();
			else if (n == 2) testRETRIEVE();
			else if (n == 3) testDELETE();
			else printf("n deve essere tra 1 e 3 compresi\n");
			return 0;
		}
	}
	return 0;
}