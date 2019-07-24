#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct object {
	char* name;
	void* value;
	size_t len;
};

int main(int argc, char *argv[]) {
	struct object testobj = {"Nome", "Jwz8e9OxnMEKPtYeSlsz", 20};
	struct object testobj2 = {"Nome2", "Prova", 5};

	int n = os_connect(argv[1]);
	printf("Connessione: %d\n", n);
	if (n == 0) {
		n = os_store(testobj.name, testobj.value, testobj.len);
		printf("Memorizzazione: %d\n", n);

		printf("Lettura: %s,\t", os_retrieve(testobj.name));
		if(strcmp((char*)testobj.value, (char*)os_retrieve(testobj.name)) == 0 ) printf("OK\n"); 
		else printf("Errore\n");

		if (os_store(testobj2.name, testobj2.value, testobj2.len) == 0) {
			n = os_delete(testobj2.name);
			printf("Rimozione: %d\n", n);
		}

		os_disconnect();
	}
	return 0;
}