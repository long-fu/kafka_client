#include <stdio.h>
#include <stdlib.h>

int create_socket(const char *ethx, const char *src_ip, int src_port, const char *dest_ip, int dest_port);
int destroy_socket(int fd);