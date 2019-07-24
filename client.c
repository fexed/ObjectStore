#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	void *contenuto = "ciao ciao ciao";

	printf("Nome: %s\nDato %s di lunghezza %d\n\n", argv[1], contenuto, 14);

	int n = os_connect(argv[1]);
	printf("Connessione: %d\n", n);
	if (n == 0) {

		n = os_store("Nome", contenuto, 14);
		printf("Memorizzazione: %d\n", n);

		printf("Lettura: %s\n", os_retrieve("Nome"));

		if (os_store("Prova", contenuto, 14) == 0) {
			n = os_delete("Prova");
			printf("Rimozione: %d\n", n);
		}

		os_disconnect();
	}
	return 0;
}