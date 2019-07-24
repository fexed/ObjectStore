#include <stdlib.h>
#define true 0
#define false 1
#define SOCKETNAME "./objstore.sock"
#define BUFFSIZE 100
#define UNIX_PATH_MAX 108

int os_connect(char *name); //Connette con username name. true se OK, false altrimenti

int os_store(char *name, void *block, size_t len);

void *os_retrieve(char *name);

int os_delete(char *name);

int os_disconnect();