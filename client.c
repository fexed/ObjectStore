#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int n = os_connect(argv[1]);
	printf("%d", n);
	os_disconnect();
	return 0;
}