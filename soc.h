#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

int socket_create(const char *ethx, const char *src_ip, int src_port, const char *dest_ip, int dest_port,struct sockaddr_in *out_dest_addr);
int socket_destroy(int fd,struct sockaddr_in *addr);