
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
#include <sys/capability.h>
#include "soc.h"

int socket_create(const char *ethx, const char *src_ip, int src_port, const char *dest_ip, int dest_port, struct sockaddr_in *out_dest_addr)
{

  cap_t caps;
  cap_value_t cap_list[2];

  // 初始化能力集
  caps = cap_get_proc();
  if (caps == NULL)
  {
    perror("cap_get_proc");
    exit(1);
  }

  // 设置 CAP_NET_RAW 和 CAP_NET_BIND_SERVICE 权限
  cap_list[0] = CAP_NET_RAW;
  cap_list[1] = CAP_NET_BIND_SERVICE;
  if (cap_set_flag(caps, CAP_PERMITTED, 2, cap_list, CAP_SET) < 0)
  {
    perror("cap_set_flag");
    exit(1);
  }
  if (cap_set_flag(caps, CAP_EFFECTIVE, 2, cap_list, CAP_SET) < 0)
  {
    perror("cap_set_flag");
    exit(1);
  }

  // 设置能力集
  if (cap_set_proc(caps) < 0)
  {
    perror("cap_set_proc");
    exit(1);
  }

  cap_free(caps);

  // 本地地址配置
  struct sockaddr_in src_addr = {0};
  // 目的地配置
  struct sockaddr_in *dest_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  out_dest_addr = dest_addr;
  dest_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  memset(dest_addr, 0, sizeof(struct sockaddr_in));

  int fd = 0;
  int ret = -1;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("create socket failed!\n");
    return -1;
  }


  inet_pton(AF_INET, src_ip, &src_addr.sin_addr.s_addr);

  src_addr.sin_family = AF_INET;
  src_addr.sin_port = htons(src_port);
  src_addr.sin_addr.s_addr = inet_addr(src_ip);

  ret = bind(fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
  if (ret < 0)
  {
    printf("bind error!!!\n");
    close(fd);
    return -1;
  }

  struct ifreq nif;
  strcpy(nif.ifr_name, ethx);
  if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&nif, sizeof(nif)) < 0)
  {
    close(fd);
    printf("bind interface fail, errno: %d \r\n", errno);
    return -1;
  }
  else
  {
    printf("bind interface success \r\n");
  }

  dest_addr->sin_family = AF_INET;
  dest_addr->sin_addr.s_addr = inet_addr(dest_ip);
  dest_addr->sin_port = htons(dest_port);

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("1.socket 网络连接失败,本线程即将终止[socket error]错误代码是%d, 错误信息是'%s'\n", errno, strerror(errno));
    setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, dest_addr, sizeof(struct sockaddr_in));
    return -1;
  }

  if (inet_pton(AF_INET, dest_ip, &dest_addr->sin_addr) <= 0)
  {
    printf("2.inet 网络连接失败,本线程即将终止[inet_pton error]错误代码是%d, 错误信息是'%s'\n", errno, strerror(errno));
    setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, dest_addr, sizeof(struct sockaddr_in));
    close(fd);
    return -1;
  }

  if (connect(fd, dest_addr, sizeof(struct sockaddr_in)) < 0)
  {
    printf("3.connect 服务器失败[connect error]错误代码是%d, 错误信息是'%s'\n", errno, strerror(errno));
    setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, dest_addr, sizeof(struct sockaddr_in));
    close(fd);
    return -1;
  }
  else
  {
    // printf("与远端建立了连接\n");
  }

  return fd;
}

int socket_destroy(int fd, struct sockaddr_in *addr)
{
  setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, addr, sizeof(struct sockaddr_in));
  free(addr);
  close(fd);
}