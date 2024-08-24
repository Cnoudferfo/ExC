#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "nic_info.h"

#define EXIT_ON_ERR(fname, skt)({ \
    printf("%s() Failed:%d\n", fname, WSAGetLastError()); \
    return -1; \
})
#define EXIT_AND_CLOSE_SOCK_ON_ERR(fname, skt)({ \
    printf("%s() Failed:%d\n", fname, WSAGetLastError()); \
    closesocket(skt); \
    return -1; \
})
#define TRY_2_P_FUNC(func, p1, p2, err, fname)({ \
    if(func(p1, p2) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TRY_3_P_FUNC(func, p1, p2, p3, err, fname)({ \
    if(func(p1, p2, p3) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TRY_4_P_FUNC(func, p1, p2, p3, p4, err, fname)({ \
    if(func(p1, p2, p3, p4) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TEST_CREATED_SOCK(csck, err, fname)({ \
    if(csck == err){ \
        EXIT_ON_ERR(fname, csck); \
    } \
})
#define TEST_RECEIVED_SOCK(rsck, err, fname)({ \
    if(rsck == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, rsck); \
    } \
})

WSADATA wsaData;
struct addrinfo *p_svr_result;

char tx_buf[32] = {"Hello, Server!"};
int  tx_buf_len = 32;
char rx_buf[32];
int  rx_buf_len = 32;

// To get server tcp address info
struct addrinfo *get_svr_addrinfo(char *ip, int port){
    struct addrinfo svr_hints;
    memset((void*)&svr_hints, 0, sizeof(svr_hints));
    svr_hints.ai_family = AF_UNSPEC;
    svr_hints.ai_socktype = SOCK_STREAM;
    svr_hints.ai_protocol = IPPROTO_TCP;
    char portStr[8];
    sprintf(portStr,"%d", (0x0000ffff & port));
    if(getaddrinfo(ip, portStr, &svr_hints, &p_svr_result) != 0){
        printf("In tcp_client(), getaddrinfo(server_info) failed:%ld\n", WSAGetLastError());
        return NULL;
    }
    return p_svr_result;
}

// An one shot tcp request
int tcp_client(struct tcp_task*pParam){
    printf("tcp_client() to ip=%s, port=%d\n", pParam->_ipv4_ip, pParam->_port_no);
    // To get server tcp address info
    struct addrinfo *p_ptr = get_svr_addrinfo(pParam->_ipv4_ip, pParam->_port_no);
    int iReturn = 0;
    // To connect to server tcp socket
    for(p_ptr=p_svr_result; p_ptr != NULL; p_ptr=p_ptr->ai_next){
        SOCKET connect_sock = socket(p_ptr->ai_family, p_ptr->ai_socktype, p_ptr->ai_protocol);
        TEST_CREATED_SOCK(connect_sock, SOCKET_ERROR, "socket");
        TRY_3_P_FUNC(connect, connect_sock, p_ptr->ai_addr, (int)p_ptr->ai_addrlen, SOCKET_ERROR, "connect");
        char *buf;
        int buf_len;
        pParam->_handler(CMD_PREP_TX_DATA, &buf, &buf_len); // To let caller to prepare tx data
        printf("tcp_client(), to send buf=%s, len=%d\n", buf, buf_len);
        TRY_4_P_FUNC(send, connect_sock, buf, buf_len, 0, SOCKET_ERROR, "send");
        // printf("tcp_client() buf=%s, %d bytes sent.\n", buf, buf_len);
        TRY_2_P_FUNC(shutdown, connect_sock, SD_SEND, SOCKET_ERROR, "shutdown");
        int iret;
        do{
            pParam->_handler(CMD_PREP_RX_BUF, &buf, &buf_len);
            iret = recv(connect_sock, buf, buf_len, 0);
            if(iret>0){
                printf("In tcp_client(){recv()}, %d bytes received.\n", iret);
                pParam->_handler(CMD_RX_DATA_READY, &buf, &buf_len);
            }
            else if(iret==0){
                printf("In tcp_client(){recv()}, to close connection.\n");
            }
            else{
                printf("In tcp_client(){recv()}, failed:%ld\n", WSAGetLastError());
                iReturn = -1;
            }
        } while(iret > 0);
        // To close the connect socket with server
        closesocket(connect_sock);
        break;  // To leave the for loop       
    }
    return iReturn;
}

int tcp_server(struct tcp_task *pParam){
    printf("tcp_server() at ip=%s, port=%d\n", pParam->_ipv4_ip, pParam->_port_no);
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // Create listening socket
    TEST_CREATED_SOCK(ListenSocket, INVALID_SOCKET, "socket"); // Check created socket
    
    struct sockaddr_in service;
    memset((void*)&service, 0, sizeof(service));
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(pParam->_ipv4_ip);
    service.sin_port = htons(pParam->_port_no);
    TRY_3_P_FUNC(bind, ListenSocket, (SOCKADDR*)&service, sizeof(service), SOCKET_ERROR, "bind");  // Bind to listening socket
    TRY_2_P_FUNC(listen, ListenSocket, SOMAXCONN, SOCKET_ERROR, "listen");  // Listen to socket
    for(int i=0;i<10;i++){  // [todo] Change this for loop to other forever-type
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);  // Inbound tcp connection arrived and clint scet was created
        TEST_RECEIVED_SOCK(ClientSocket, INVALID_SOCKET, "accept");  // Check created socket
        char *buf;
        int  bufLen;
        int iret = pParam->_handler(CMD_PREP_RX_BUF, &buf, &bufLen);  // To let caller prepare rx buffer
        // Receive until the peer shuts down the connection
        int icnt;
        do {
            icnt = recv(ClientSocket, buf, bufLen, 0);    // To receive data
            if(icnt > 0) {
                printf("Bytes received: %d\n", icnt);
                iret = pParam->_handler(CMD_RX_DATA_READY, &buf, &bufLen); // To pass rx data back to caller 
                iret = pParam->_handler(CMD_PREP_TX_DATA, &buf, &bufLen);  // To let caller prepare tx data
                TRY_4_P_FUNC(send, ClientSocket, buf, bufLen, 0, SOCKET_ERROR, "send"); // To send data
                printf("Bytes sent: %d\n", bufLen);
            }
            else if (icnt == 0) {
                printf("Connection closing...\n");
                TRY_2_P_FUNC(shutdown, ClientSocket, SD_SEND, SOCKET_ERROR, "shutdown");
                closesocket(ClientSocket);  // cleanup
            }
            else  {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                return -1;
            }
        } while (icnt > 0);
    }
    // no longer need server socket
    closesocket(ListenSocket);
    return 0;
}
int tcp_init(void){
    return WSAStartup(MAKEWORD(2,2), &wsaData);
}
int tcp_destroy(void){
    return WSACleanup();
}