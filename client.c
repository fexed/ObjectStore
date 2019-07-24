#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct object {
	char* name;
	void* value;
	size_t len;
};

struct object testobj = {"Nome", "Jwz8e9OxnMEKPtYeSlsz", 21};
struct object testobj2 = {"Nome2", "Prova", 5};

int testSTORE() {
	int n = os_store(testobj.name, testobj.value, testobj.len);
			os_disconnect();
	return n;
	//printf("Memorizzazione: %d\n", n);
}

int testRETRIEVE() {
	int n = strcmp((char*)testobj.value, (char*)os_retrieve(testobj.name));
			os_disconnect();
	return n;
}

int testDELETE() {
	int n = os_store(testobj2.name, testobj2.value, testobj2.len);
	if (n == 0) {
		n =  os_delete(testobj2.name);
			os_disconnect();
		return n;
	} else {
		os_disconnect();
		return n;
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

			if (n == 1) return testSTORE();
			else if (n == 2) return testRETRIEVE();
			else if (n == 3) return testDELETE();
			else printf("n deve essere tra 1 e 3 compresi\n");
		} else return -2;
	}
	return 0;
}