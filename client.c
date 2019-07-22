#include "libclient.h"

int main(int argc, char *argv[]) {
	int n = os_connect(argv[1]);
	printf("%d", n);

	return n;
}