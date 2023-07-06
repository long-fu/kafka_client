#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

int socket_create(const char *ethx, const char *src_ip, int src_port, const char *dest_ip, int dest_port);
int socket_destroy(int fd);