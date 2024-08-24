#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "socket_info.h"
#include "macros.h"

WSADATA wsaData;
struct addrinfo *p_svr_result;

char tx_buf[32] = {"Hello, Server!"};
int  tx_buf_len = 32;
char rx_buf[32];
int  rx_buf_len = 32;

// An one shot tcp request
// int tcp_client(struct tcp_task*pParam){
int tcp_client(struct a_sock *p_as){
    struct heap_tcp_client *p_hp = &(p_as->_heap._tcp_cli);
    // To get server tcp address info
    memset((void*)&(p_hp->_svr_hints), 0, sizeof(p_hp->_svr_hints));
    p_hp->_svr_hints.ai_family = AF_UNSPEC;
    p_hp->_svr_hints.ai_socktype = SOCK_STREAM;
    p_hp->_svr_hints.ai_protocol = IPPROTO_TCP;
    sprintf(p_hp->_portStr,"%d", (0x0000ffff & p_as->_nic_info._portNo));
    if(getaddrinfo(p_as->_nic_info._ipv4._ip_addr, p_hp->_portStr, &(p_hp->_svr_hints), &(p_hp->_p_svr_result)) != 0){
        printf("In tcp_client(), getaddrinfo(server_info) failed:%ld\n", WSAGetLastError());
        return -1;
    }
    
    // To connect to server tcp socket
    for(p_hp->_p_ptr=p_hp->_p_svr_result; p_hp->_p_ptr != NULL; p_hp->_p_ptr=p_hp->_p_ptr->ai_next){
        p_hp->_connectSock = socket(p_hp->_p_ptr->ai_family, p_hp->_p_ptr->ai_socktype, p_hp->_p_ptr->ai_protocol);
        TEST_CREATED_SOCK(p_hp->_connectSock, SOCKET_ERROR, "socket");
        TRY_3_P_FUNC(connect, p_hp->_connectSock, p_hp->_p_ptr->ai_addr, (int)(p_hp->_p_ptr->ai_addrlen), SOCKET_ERROR, "connect");
        p_as->_IF_handler(CMD_PREP_TX_DATA, &(p_hp->_buf), &(p_hp->_bufLen)); // To let caller to prepare tx data
        TRY_4_P_FUNC(send, p_hp->_connectSock, p_hp->_buf, p_hp->_bufLen, 0, SOCKET_ERROR, "send");
        TRY_2_P_FUNC(shutdown, p_hp->_connectSock, SD_SEND, SOCKET_ERROR, "shutdown");
        
        do{
            p_as->_IF_handler(CMD_PREP_RX_BUF, &(p_hp->_buf), &(p_hp->_bufLen));
            p_hp->_nbytes = recv(p_hp->_connectSock, p_hp->_buf, p_hp->_bufLen, 0);
            if(p_hp->_nbytes>0){
                printf("In tcp_client(){recv()}, %d bytes received.\n", p_hp->_nbytes);
                p_as->_IF_handler(CMD_RX_DATA_READY, &(p_hp->_buf), &(p_hp->_bufLen));
            }
            else if(p_hp->_nbytes==0){
                printf("In tcp_client(){recv()}, to close connection.\n");
            }
            else{
                printf("In tcp_client(){recv()}, failed:%ld\n", WSAGetLastError());
                closesocket(p_hp->_connectSock);
                return -1;
            }
        } while(p_hp->_nbytes > 0);
        // To close the connect socket with server
        closesocket(p_hp->_connectSock);
        break;  // To leave the for loop       
    }
    return 0;
}

// int tcp_server(struct tcp_task *pParam, int in_flg){
int tcp_server(struct a_sock *p_as, int in_flg){
    struct heap_tcp_server *p_hp = &(p_as->_heap._tcp_svr);
    p_hp->_flag = in_flg;
    p_hp->_listenSock = INVALID_SOCKET;
    p_hp->_clientSock = INVALID_SOCKET;
    p_hp->_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // Create listening socket
    TEST_CREATED_SOCK(p_hp->_listenSock, INVALID_SOCKET, "socket"); // Check created socket
    memset((void*)&(p_hp->_localAddr), 0, sizeof(p_hp->_localAddr)); // To fill out server's address information
    p_hp->_localAddr.sin_family = AF_INET;
    p_hp->_localAddr.sin_addr.s_addr = inet_addr(p_as->_nic_info._ipv4._ip_addr);
    p_hp->_localAddr.sin_port = htons(p_as->_nic_info._portNo);
    TRY_3_P_FUNC(bind, p_hp->_listenSock, (SOCKADDR*)&(p_hp->_localAddr), sizeof(p_hp->_localAddr), SOCKET_ERROR, "bind");  // To bind to listening socket
    TRY_2_P_FUNC(listen, p_hp->_listenSock, SOMAXCONN, SOCKET_ERROR, "listen");  // Listen to socke
    do{
        p_hp->_clientSock = accept(p_hp->_listenSock, NULL, NULL);  // Inbound tcp connection arrived and clint socket was created
        TEST_RECEIVED_SOCK(p_hp->_clientSock, INVALID_SOCKET, "accept");  // To check created socket
        // To let caller prepare rx / tx buffer : buffer pointer and buffer length
        // Calling convention of nbytes = pParam->handler(<command>, <pointer to buffer>, <pointer to buffer length>)
        // Return value: only care about when <command> is CMD_RX_DATA_RDY
        //               return to exit this server function when nbytes == -1 
        p_hp->_iret = p_as->_IF_handler(CMD_PREP_RX_BUF, &(p_hp->_buf), &(p_hp->_bufLen));  // To let caller prepare rx buffer
        do {
            p_hp->_nbytes = recv(p_hp->_clientSock, p_hp->_buf, p_hp->_bufLen, 0);    // To receive data
            if(p_hp->_nbytes > 0) {
                printf("Bytes received: %d\n", p_hp->_nbytes);
                p_hp->_iret = p_as->_IF_handler(CMD_RX_DATA_READY, &(p_hp->_buf), &(p_hp->_bufLen)); // To pass rx data back to caller
                if(p_hp->_iret == -1){    // To exit this server function
                    closesocket(p_hp->_clientSock);  // close client socket
                    closesocket(p_hp->_listenSock);  // close listen socket
                    printf("tcp_server() to return...\n");
                    return 0;
                }
                p_hp->_iret = p_as->_IF_handler(CMD_PREP_TX_DATA, &(p_hp->_buf), &(p_hp->_bufLen));  // To let caller prepare tx data
                TRY_4_P_FUNC(send, p_hp->_clientSock, p_hp->_buf, p_hp->_bufLen, 0, SOCKET_ERROR, "send"); // To send data
                printf("Bytes sent: %d\n", p_hp->_bufLen);
            }
            else if (p_hp->_nbytes == 0) {
                printf("Connection closing...\n");
                TRY_2_P_FUNC(shutdown, p_hp->_clientSock, SD_SEND, SOCKET_ERROR, "shutdown");
                closesocket(p_hp->_clientSock);
            }
            else  {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(p_hp->_clientSock);
                return -1;
            }
        } while (p_hp->_nbytes > 0);
    }while(p_hp->_flag == FLAG_FOREVER);
    closesocket(p_hp->_listenSock);
    return 0;
}