#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/capability.h>

#define TCP_ADDR "22.10.133.22"
#define TCP_PORT 7890
#define SEND_PORT 7888
#define MAC_SIZE 18
#define IP_SIZE 16
#define ETHX "enp0s31f6"


// 获取本机mac
int get_local_mac(const char *eth_inf, char *mac) {
  struct ifreq ifr;
  int sd;

  bzero(&ifr, sizeof(struct ifreq));
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("get %s mac address socket creat error\n", eth_inf);
    return -1;
  }

  strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);

  if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0) {
    printf("get %s mac address error\n", eth_inf);
    close(sd);
    return -1;
  }

  snprintf(mac, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
           (unsigned char)ifr.ifr_hwaddr.sa_data[0],
           (unsigned char)ifr.ifr_hwaddr.sa_data[1],
           (unsigned char)ifr.ifr_hwaddr.sa_data[2],
           (unsigned char)ifr.ifr_hwaddr.sa_data[3],
           (unsigned char)ifr.ifr_hwaddr.sa_data[4],
           (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

  close(sd);

  return 0;
}
// 获取本机ip
int get_local_ip(const char *eth_inf, char *ip) {
  int sd;
  struct sockaddr_in sin;
  struct ifreq ifr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (-1 == sd) {
    printf("socket error: %s\n", strerror(errno));
    return -1;
  }

  strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ - 1] = 0;

  // if error: No such device
  if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    close(sd);
    return -1;
  }

  memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
  snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

  close(sd);
  return 0;
}
// 获取本机网卡名称
int get_local_dev(char *eth_name, const char *ip) {
  
  int sock;
  struct sockaddr_in sin;
  struct ifreq ifr;
  struct ifconf ifc;
  char buf[1024];
  int i;

  // 创建一个套接字
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    perror("socket error");
    return -1;
  }
  // 获取系统中所有网卡的信息
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
    perror("ioctl error");
    return -1;
  }

  // 遍历所有网卡，获取每个网卡的 IP 地址
  for (i = 0; i < ifc.ifc_len;) {
    struct ifreq *pifr = (struct ifreq *)(buf + i);

    // 调用 ioctl 函数获取网卡 IP 地址
    if (ioctl(sock, SIOCGIFADDR, pifr) < 0) {
      perror("ioctl error");
      return -1;
    }

    // 如果网卡 IP 地址与输入的 IP 地址相同，则该网卡就是输入的 IP 地址对应的

    if (strcmp(inet_ntoa(((struct sockaddr_in *)&pifr->ifr_addr)->sin_addr),ip) == 0) {
      strcpy(eth_name,pifr->ifr_name);
      break;
    }
    // 移动指针到下一个网卡
    i += sizeof(struct ifreq);
  }

  close(sock);
  return 0;
}


int main(int argc, char* argv[])
{
	cap_t caps;
	cap_value_t cap_list[2];

	struct sockaddr_in addr, mcast_addr;
	int fd = 0;
	struct ip_mreq ipmr;
	char ip[IP_SIZE];
	int ret = -1;
	char chrUDP[124] = {"1234567890abcdefghijklmnopqrstuvwxyz"};
	char dev_name[100] = {0};
	int len;

    // 初始化能力集
    caps = cap_get_proc();
    if (caps == NULL) {
        perror("cap_get_proc");
        exit(1);
    }

    // 设置 CAP_NET_RAW 和 CAP_NET_BIND_SERVICE 权限
    cap_list[0] = CAP_NET_RAW;
    cap_list[1] = CAP_NET_BIND_SERVICE;
    if (cap_set_flag(caps, CAP_PERMITTED, 2, cap_list, CAP_SET) < 0) {
        perror("cap_set_flag");
        exit(1);
    }
    if (cap_set_flag(caps, CAP_EFFECTIVE, 2, cap_list, CAP_SET) < 0) {
        perror("cap_set_flag");
        exit(1);
    }

    // 设置能力集
    if (cap_set_proc(caps) < 0) {
        perror("cap_set_proc");
        exit(1);
    }

    cap_free(caps);

    // 在此之后的代码中程序将拥有 CAP_NET_RAW 和 CAP_NET_BIND_SERVICE 权限
    // ......

	get_local_dev(dev_name, "22.10.133.111");

	printf("The device name is: %s\n", dev_name);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create socket failed!");
		return -1;
	}

	get_local_ip(ETHX, ip);
	printf("local %s ip: %s\n", ETHX, ip);
	inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SEND_PORT);
	// addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_addr.s_addr = inet_addr(ip);
	ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		printf("bind error!!!");
		perror("bind:");
		close(fd);
		return -1;
	}
	/* 指定接口 绑定接口 */
	struct ifreq nif;
	strcpy(nif.ifr_name, ETHX);
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&nif, sizeof(nif)) < 0) {
		close(fd);
		printf("bind interface fail, errno: %d \r\n", errno);
		return -1;
	} else {
		printf("bind interface success \r\n");
	}

	memset(&mcast_addr, 0, sizeof(mcast_addr));
	mcast_addr.sin_family = AF_INET;
	mcast_addr.sin_addr.s_addr = inet_addr(TCP_ADDR);
	mcast_addr.sin_port = htons(TCP_PORT);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            printf("1.socket 网络连接失败,本线程即将终止[socket error]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            
    }

    if (inet_pton(AF_INET, TCP_ADDR,  &mcast_addr.sin_addr) <= 0 ){
            printf("2.inet 网络连接失败,本线程即将终止[inet_pton error]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            close(fd);
            
    }

    if (connect(fd, (struct sockaddr *)&mcast_addr, sizeof(mcast_addr)) < 0){
            printf("3.connect 服务器失败[connect error]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            close(fd);
            
    }
    else
        printf("与远端建立了连接\n");

	int length = 1000;
    char *body = (char *)malloc(length+500);
    char *head = (char *)malloc(length+1000);

    memset(body, 0, length+500);
    memset(head, 0, length+1000);

    sprintf(body, "{\"CameraName\":\"111\",\"SiteData\":{\"Latitude\":\"16.24463,44.179439\",\"Longitude\":\"001\",\"Name\":\"001\"},\"ChannelName\":\"\",\"AlarmTime\":\"444\",\"AlgCode\":\"222\",\"DeviceId\":\"333\",\"AlarmBoxs\":[{\"X\":1236,\"Y\":545,\"Height\":529,\"Width\":234},{\"X\":1419,\"Y\":509,\"Height\":337,\"Width\":126},{\"X\":1203,\"Y\":545,\"Height\":388,\"Width\":123}],\"AlarmExtension\":\"555\",\"ChannelId\":\"eb5d32\",\"AlarmBase\":\"666\"}");

    char *str=(char *)malloc(128);
    len = strlen(body);
    sprintf(str, "%d", len);

    strcat(head, "POST /api/smartbox/AlarmPost HTTP/1.1\r\n");
    strcat(head, "Cache-Control:no-cache\r\n");
    strcat(head, "Connection:Keep-Alive\r\n");
    strcat(head, "Accept-Encoding:gzip,deflate,br\r\n");
    strcat(head, "Accept:*/*\r\n");
    strcat(head, "Content-Type:application/json\r\n");
    strcat(head, "User-Agent:Mozilla/5.0\r\n");
    strcat(head, "host:192.168.137.220\r\n");
    strcat(head, "Content-Length:");
    strcat(head, str);
    strcat(head, "\r\n\r\n");
    //body的值为post的数据
    strcat(head, body);
    strcat(head, "\r\n\r\n");
//    printf("%s\n",head);

    ret = write(fd,head,strlen(head));
    if (ret < 0) {
        printf("4.send 发送失败！[write]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
        close(fd);
        free(body);
        free(head);
        free(str);
        body = NULL;
        head = NULL;
        str = NULL;
        
    }else{
        printf("消息发送成功，共发送了%d个字节! \n\n", ret);
    }

	setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mcast_addr, sizeof(mcast_addr));
	close(fd);

  return 0;

}