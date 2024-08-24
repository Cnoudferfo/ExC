// Client.c:
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int print_result(struct addrinfo*);

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    const char *sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    SOCKET MySocket = INVALID_SOCKET;
    struct sockaddr_in my_addr;
    struct ip_mreq mcast_req;
    int addrlen;
    int nbytes;
    char buffer[32];
    
    // Validate the parameters
    if (argc != 3) {
        printf("usage: %s server-ip client-ip\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Bind me here
    // Bind to a datagram socket
    MySocket = socket( AF_INET, SOCK_DGRAM, 0);
    if(MySocket == INVALID_SOCKET){
        printf("socket() failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    // Bind
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr(argv[2]);
    my_addr.sin_port = htons(27066);
    iResult = bind(MySocket, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(iResult == SOCKET_ERROR){
        printf("bind() failed with error:%ld\n", WSAGetLastError());
        closesocket(MySocket);
        WSACleanup();
        return 1;
    }

    // Join a multicast group
    mcast_req.imr_multiaddr.s_addr = inet_addr("239.0.0.1");
    mcast_req.imr_interface.s_addr = inet_addr(argv[2]);
    iResult = setsockopt(MySocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast_req, sizeof(mcast_req));
    if(iResult == SOCKET_ERROR){
        printf("setsockopt() failed with error:%ld\n", WSAGetLastError());
        closesocket(MySocket);
        WSACleanup();
        return 1;
    }

    // Receive message from mcast
    addrlen = sizeof(my_addr);
    nbytes = recvfrom(MySocket, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&my_addr, &addrlen);
    if(nbytes == SOCKET_ERROR){
        printf("recvfrom() failed with error:%ld\n", WSAGetLastError());
        closesocket(MySocket);
        WSACleanup();
        return 1;
    }
    buffer[nbytes] = '\0';
    printf("Mcast received:%s\n", buffer);

    // Leave the mcast group
    setsockopt(MySocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mcast_req, sizeof(mcast_req));

    // Resolve the server address and port
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    iResult = getaddrinfo(buffer, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    printf("print_result()=%d\n", print_result(result));

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

//To print addrinfo
int print_result(struct addrinfo *pResult)
{
    struct addrinfo *ptr = NULL;
    struct sockaddr_in *sockaddr_ipv4;
    LPSOCKADDR sockaddr_ip;
    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;
    INT iRetval;
    int i = 1;
    u_short port_number;

// Retrieve each address and print out the hex bytes
    for(ptr=pResult; ptr != NULL ;ptr=ptr->ai_next) {

        printf("getaddrinfo response %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tFamily: ");
        switch (ptr->ai_family) {
            case AF_UNSPEC:
                printf("Unspecified\n");
                break;
            case AF_INET:
                printf("AF_INET (IPv4)\n");
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                port_number = htons(((struct sockaddr_in*)(ptr->ai_addr))->sin_port);
                printf("\tIPv4 address %s:%d\n",
                    inet_ntoa(sockaddr_ipv4->sin_addr), port_number);
                break;
            case AF_INET6:
                printf("AF_INET6 (IPv6)\n");
                // the InetNtop function is available on Windows Vista and later
                // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                // printf("\tIPv6 address %s\n",
                //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );
                
                // We use WSAAddressToString since it is supported on Windows XP and later
                sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
                    ipstringbuffer, &ipbufferlength );
                if (iRetval)
                    printf("WSAAddressToString failed with %u\n", WSAGetLastError() );
                else    
                    printf("\tIPv6 address %s\n", ipstringbuffer);
                break;
            case AF_NETBIOS:
                printf("AF_NETBIOS (NetBIOS)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_family);
                break;
        }
        printf("\tSocket type: ");
        switch (ptr->ai_socktype) {
            case 0:
                printf("Unspecified\n");
                break;
            case SOCK_STREAM:
                printf("SOCK_STREAM (stream)\n");
                break;
            case SOCK_DGRAM:
                printf("SOCK_DGRAM (datagram) \n");
                break;
            case SOCK_RAW:
                printf("SOCK_RAW (raw) \n");
                break;
            case SOCK_RDM:
                printf("SOCK_RDM (reliable message datagram)\n");
                break;
            case SOCK_SEQPACKET:
                printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_socktype);
                break;
        }
        printf("\tProtocol: ");
        switch (ptr->ai_protocol) {
            case 0:
                printf("Unspecified\n");
                break;
            case IPPROTO_TCP:
                printf("IPPROTO_TCP (TCP)\n");
                break;
            case IPPROTO_UDP:
                printf("IPPROTO_UDP (UDP) \n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_protocol);
                break;
        }
        printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
        printf("\tCanonical name: %s\n", ptr->ai_canonname);
    }    
    return 0;
}