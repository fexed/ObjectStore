#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int n = os_connect(argv[1]);
	printf("Connessione: %d\n", n);
	if (n == 0) {
		n = os_store("Nome", "Contenuto", 10);
		printf("Memorizzazione: %d\n", n);

		os_disconnect();
	}
	return 0;
}