// Client.c:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MULTICAST_GROUP "239.0.0.1"
#define MULTICAST_PORT 12345
#define SERVER_PORT 54321

int main() {
    int sockfd;
    struct sockaddr_in addr;
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(MULTICAST_PORT);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    int len = sizeof(addr);
    int received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &len);
    if (received < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    buffer[received] = '\0';
    printf("Received multicast message: %s\n", buffer);

    // Here you would extract the IP and port from the message and use it to connect via TCP
    // For simplicity, this example will just connect to a predefined server port

    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with actual server IP

    if (connect(tcp_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char *tcp_message = "Hello, Server!";
    send(tcp_sockfd, tcp_message, strlen(tcp_message), 0);

    printf("Sent TCP message to server: %s\n", tcp_message);

    close(tcp_sockfd);
    close(sockfd);
    return 0;
}
