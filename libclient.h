#include <stdlib.h>
#define SOCKETNAME "./objstore.sock"
#define BUFFSIZE 100
#define UNIX_PATH_MAX 108
#define true 0
#define false 1

int os_connect(char *name);

int os_store(char *name, void *block, size_t len);

void *os_retrieve(char *name);

int os_delete(char *name);

int os_disconnect();