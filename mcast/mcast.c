// For Windows
// Rx
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MULTICAST_GROUP "239.0.0.1"
#define PORT 12345

int mcast_rx_main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in local_addr;
    struct ip_mreq multicast_req;
    char buffer[256];
    int nbytes;
    int addrlen;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup");
        exit(EXIT_FAILURE);
    }

    // Create a socket for receiving data
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Initialize the local address structure
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(PORT);
    
    // Bind the socket to the local address
    if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
        perror("bind");
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Join the multicast group
    multicast_req.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&multicast_req, sizeof(multicast_req)) == SOCKET_ERROR) {
        perror("setsockopt");
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Receive the message
    addrlen = sizeof(local_addr);
    nbytes = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&local_addr, &addrlen);
    if (nbytes == SOCKET_ERROR) {
        perror("recvfrom");
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    buffer[nbytes] = '\0';
    printf("Received message: %s\n", buffer);
    
    // Leave the multicast group
    setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&multicast_req, sizeof(multicast_req));

    // Close the socket
    closesocket(sock);
    WSACleanup();
    
    return 0;
}

// Tx
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <winsock2.h>
// #include <ws2tcpip.h>

// #define MULTICAST_GROUP "239.0.0.1"
// #define PORT 12345

int mcast_tx_main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in multicast_addr;
    char *message = "Hello, Multicast!";
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup");
        exit(EXIT_FAILURE);
    }

    // Create a socket for sending data
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Initialize the multicast address structure
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_addr.sin_port = htons(PORT);
    
    // Send the message
    if (sendto(sock, message, strlen(message), 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) == SOCKET_ERROR) {
        perror("sendto");
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    printf("Message sent to multicast group %s\n", MULTICAST_GROUP);
    
    // Close the socket
    closesocket(sock);
    WSACleanup();
    
    return 0;
}
