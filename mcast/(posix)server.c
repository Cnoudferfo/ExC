// Server.c:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MULTICAST_GROUP "239.0.0.1"
#define MULTICAST_PORT 12345

int main() {
    int sockfd;
    struct sockaddr_in addr;
    char *message = "Server IP and Port";

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    addr.sin_port = htons(MULTICAST_PORT);

    while (1) {
        int sent = sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&addr, sizeof(addr));
        if (sent < 0) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }

        printf("Sent multicast message: %s\n", message);
        sleep(1);
    }

    close(sockfd);
    return 0;
}
