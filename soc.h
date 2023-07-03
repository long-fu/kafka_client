#include <stdio.h>
#include <stdlib.h>

int get_local_dev(char *eth_name, const char *ip);
int create_socket(const char *ethx, const char *src_ip, int src_port, const char *dest_ip, int dest_port);
int destroy_socket(int fd);