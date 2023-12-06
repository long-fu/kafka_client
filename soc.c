
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "soc.h"

int socket_create(const char *ethx, const char *src_ip, int src_port, const char *dest_ip, int dest_port)
{

  int sock = -1;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    // printf("socket create failed");
    printf("[socket error] code:'%d' msg:'%s'\n" ,errno, strerror(errno));
    return -1;
  }

  // struct ifreq interface = {0};
  // memset(interface.ifr_ifrn.ifrn_name,0x0, IFNAMSIZ);
  // strncpy(interface.ifr_name, ethx, strlen(ethx));

  const struct ifreq interface = {
      .ifr_name = ethx,
  };

  if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &interface, sizeof(interface)) < 0)
  {
    printf("[setsockopt SO_BINDTODEVICE error] code:'%d' msg:'%s'\n" ,errno, strerror(errno));
  }

  struct sockaddr_in servaddr = {0};
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(dest_port);
  servaddr.sin_addr.s_addr = inet_addr(dest_ip);

  if (connect(sock, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    // fprintf(stderr, "connection to the server failed...\n");
    printf("[connect error] code:'%d' msg:'%s'\n" ,errno, strerror(errno));
    close(sock);
    return -1;
  }

  return sock;
}

int socket_destroy(int sock)
{
  close(sock);
}