// server.c
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(void) 
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    // struct addrinfo *result = NULL;
    // struct addrinfo hints;
    struct sockaddr_in service;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int i;
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    else{
        printf("WSAStartup() returned=%d\n",iResult);
    }

    // ZeroMemory(&hints, sizeof(hints));
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_protocol = IPPROTO_TCP;
    // hints.ai_flags = AI_PASSIVE;

    // // Resolve the server address and port
    // iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    // if ( iResult != 0 ) {
    //     printf("getaddrinfo failed with error: %d\n", iResult);
    //     WSACleanup();
    //     return 1;
    // }
    // else{
    //     printf("getaddrinfo() returned=%d\n", iResult);
    // }

    // Create a SOCKET for the server to listen for client connections.
    // ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        // freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    // else{
    //     printf("socket() returned=%d\n", iResult);
    // }
    
    // Setup the TCP listening socket
    // iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("30.30.30.1");
    service.sin_port = htons(27015);
    iResult = bind(ListenSocket, (SOCKADDR*)&service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        // freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // freeaddrinfo(result);
    
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else{
        printf("listen() returned=%d\n", iResult);
    }
    
    for(i=0;i<10;i++){
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        else{
            printf("%d-th accept() returned=%d\n", i, iResult);
        }

        // Receive until the peer shuts down the connection
        do {
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                printf("Bytes received: %d\n", iResult);

                // Echo the buffer back to the sender
                iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
                printf("Bytes sent: %d\n", iSendResult);
            }
            else if (iResult == 0) {
                printf("Connection closing...\n");
                //shutdown the connection since we're done
                iResult = shutdown(ClientSocket, SD_SEND);
                if (iResult == SOCKET_ERROR) {
                    printf("shutdown failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
                // cleanup
                closesocket(ClientSocket);
            }
            else  {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        } while (iResult > 0);
    }

    // no longer need server socket
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}