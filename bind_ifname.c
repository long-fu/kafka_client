#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>

int main(void)
{
    const int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    const struct ifreq ifr = {
        .ifr_name = "enp49s0",
    };

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
        perror("setsockopt");
        return EXIT_FAILURE;
    }

    const struct sockaddr_in servaddr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = inet_addr("192.168.2.2"),
        .sin_port        = htons(80),
    };

    if (connect(sockfd, (const struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "Connection to the server failed...\n");
        return EXIT_FAILURE;
    }

    // Make an HTTP request to Google
    dprintf(sockfd, "GET / HTTP/1.1\r\n");
    dprintf(sockfd, "HOST: www.google.com\r\n");
    dprintf(sockfd, "\r\n");

    char buffer[1024*1024] = {};
    read(sockfd, buffer, sizeof(buffer) - 1);

    printf("Response: '%s'\n", buffer);

    close(sockfd);
    return EXIT_SUCCESS;
}