#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct object {
	char* name;
	void* value;
	size_t len;
};

struct object testobj = {"Nome", "Jwz8e9OxnMEKPtYeSlsz", 20};
struct object testobj2 = {"Nome2", "Prova", 5};

void testSTORE() {
	int n;
	n = os_store(testobj.name, testobj.value, testobj.len);
	printf("Memorizzazione: %d\n", n);
}

void testRETRIEVE() {
	printf("Lettura: %s,\t", os_retrieve(testobj.name));
	if(strcmp((char*)testobj.value, (char*)os_retrieve(testobj.name)) == 0 ) printf("OK\n"); 
	else printf("Errore\n");
}

void testDELETE() {
	int n;
	if (os_store(testobj2.name, testobj2.value, testobj2.len) == 0) {
		n = os_delete(testobj2.name);
		printf("Rimozione: %d\n", n);
	}	
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: %s name n\n", argv[0]);
		return -1;
	} else {
		int n = os_connect(argv[1]);

		if (n == 0) {
			n = atoi(argv[2]);

			if (n == 1) testSTORE();
			else if (n == 2) testRETRIEVE();
			else if (n == 3) testDELETE();
			else printf("n deve essere tra 1 e 3 compresi\n");
			os_disconnect();
		} else return -2;
	}
	return 0;
}