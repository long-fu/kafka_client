
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

// #define MAC_SIZE 18
// #define IP_SIZE 16

// // 获取本机mac
// int get_local_mac(const char *eth_inf, char *mac)
// {
//   struct ifreq ifr;
//   int sd;

//   bzero(&ifr, sizeof(struct ifreq));
//   if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
//   {
//     printf("get %s mac address socket creat error\n", eth_inf);
//     return -1;
//   }

//   strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);

//   if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
//   {
//     printf("get %s mac address error\n", eth_inf);
//     close(sd);
//     return -1;
//   }

//   snprintf(mac, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
//            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

//   close(sd);

//   return 0;
// }

// // 获取本机ip
// int get_local_ip(const char *eth_inf, char *ip)
// {
//   int sd;
//   struct sockaddr_in sin;
//   struct ifreq ifr;

//   sd = socket(AF_INET, SOCK_DGRAM, 0);
//   if (-1 == sd)
//   {
//     printf("socket error: %s\n", strerror(errno));
//     return -1;
//   }

//   strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
//   ifr.ifr_name[IFNAMSIZ - 1] = 0;

//   // if error: No such device
//   if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
//   {
//     printf("ioctl error: %s\n", strerror(errno));
//     close(sd);
//     return -1;
//   }

//   memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
//   snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

//   close(sd);
//   return 0;
// }

// // 获取本机网卡名称
// int get_local_dev(char *eth_name, const char *ip)
// {

//   int sock;
//   struct sockaddr_in sin;
//   struct ifreq ifr;
//   struct ifconf ifc;
//   char *buf = (char *)malloc(1024);
//   memset(buf, 0x0, 1024);
//   //   char buf[10240];
//   int i;

//   // 创建一个套接字
//   sock = socket(AF_INET, SOCK_DGRAM, 0);
//   if (sock == -1)
//   {
//     perror("socket error");
//     return -1;
//   }
//   // 获取系统中所有网卡的信息
//   ifc.ifc_len = 1024;
//   ifc.ifc_buf = buf;
//   if (ioctl(sock, SIOCGIFCONF, &ifc) < 0)
//   {
//     perror("ioctl error");
//     return -1;
//   }

//   // 遍历所有网卡，获取每个网卡的 IP 地址
//   for (i = 0; i < ifc.ifc_len;)
//   {
//     struct ifreq *pifr = (struct ifreq *)(buf + i);

//     // 调用 ioctl 函数获取网卡 IP 地址
//     if (ioctl(sock, SIOCGIFADDR, pifr) < 0)
//     {
//       perror("ioctl error");
//       return -1;
//     }

//     // 如果网卡 IP 地址与输入的 IP 地址相同，则该网卡就是输入的 IP 地址对应的
//     if (strcmp(inet_ntoa(((struct sockaddr_in *)&pifr->ifr_addr)->sin_addr), ip) == 0)
//     {
//       strcpy(eth_name, pifr->ifr_name);
//       break;
//     }
//     // 移动指针到下一个网卡
//     i += sizeof(struct ifreq);
//   }
//   free(buf);
//   close(sock);
//   return 0;
// }

int create_socket(const char *ethx, const char *src_ip, int src_port, const char *dest_ip, int dest_port)
{

  cap_t caps;
  cap_value_t cap_list[2];

  // 本地地址配置
  struct sockaddr_in src_addr = {0};
  // 目的地配置
  struct sockaddr_in dest_addr = {0};

  int fd = 0;
  int ret = -1;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("create socket failed!\n");
    return -1;
  }

  inet_pton(AF_INET, src_ip, &src_addr.sin_addr.s_addr);
  memset(&src_addr, 0, sizeof(src_addr));
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
    return -1;
  }

  if (inet_pton(AF_INET, dest_ip, dest_addr->sin_addr) <= 0)
  {
    printf("2.inet 网络连接失败,本线程即将终止[inet_pton error]错误代码是%d, 错误信息是'%s'\n", errno, strerror(errno));
    close(fd);
    return -1;
  }

  if (connect(fd, (struct sockaddr *)dest_addr, sizeof(struct sockaddr_in)) < 0)
  {
    printf("3.connect 服务器失败[connect error]错误代码是%d, 错误信息是'%s'\n", errno, strerror(errno));
    close(fd);
    return -1;
  }
  else
  {
    printf("与远端建立了连接\n");
  }

  return fd;
}

int destroy_socket(int fd,struct sockaddr_in *addr)
{
  setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, addr, sizeof(struct sockaddr_in));
  close(fd);
}