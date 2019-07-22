#include "libclient.h"
#include <stdlib.h>
#include <stdio.h>

int os_connect(char *name){
	return true;
}

int os_store(char *name, void *block, size_t len);

void *os_retrieve(char *name);

int os_delete(char *name);

int os_disconnect();