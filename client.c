#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int n = os_connect(argv[1]);
	printf("Connessione: %d\n", n);
	if (n == 0) {
		n = os_store("Nome", "Contenuto in più", 17);
		printf("Memorizzazione: %d\n", n);

		printf("Lettura: %s\n", (char *) os_retrieve("Nome"));

		if (os_store("Prova", "Contenuto in più", 17) == 0) {
			n = os_delete("Prova");
			printf("Rimozione: %d\n", n);
		}

		os_disconnect();
	}
	return 0;
}