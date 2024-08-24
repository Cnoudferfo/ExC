// For Windows
// Rx
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <synchapi.h>
#include "socket_info.h"
#include "macros.h"

WSADATA wsaData;

int mcast_rx_main(struct a_sock * p_as) {
    struct heap_mcast_rx *p_hp = &(p_as->_heap._mcast_rx);
    p_hp->_sock = socket(AF_INET, SOCK_DGRAM, 0);  // Create a socket for receiving data
    TEST_CREATED_SOCK(p_hp->_sock, INVALID_SOCKET, "socket");
  
    memset(&(p_hp->_localAddr), 0, sizeof(p_hp->_localAddr));  // Initialize the local address structure
    p_hp->_localAddr.sin_family = AF_INET;
    p_hp->_localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    p_hp->_localAddr.sin_port = htons(p_as->_nic_info._portNo);
    
    TRY_3_P_FUNC(bind, p_hp->_sock, (SOCKADDR*)&(p_hp->_localAddr), sizeof(p_hp->_localAddr), SOCKET_ERROR, "bind");  // Bind the socket to the local address
    
    p_hp->_mcastReq.imr_multiaddr.s_addr = inet_addr(p_as->_nic_info._ipv4._mcast_grp);  // Join the multicast group    
    p_hp->_mcastReq.imr_interface.s_addr = htonl(INADDR_ANY);
    TRY_5_P_FUNC(setsockopt, p_hp->_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&(p_hp->_mcastReq), sizeof(p_hp->_mcastReq), SOCKET_ERROR, "setsockopt");

    p_as->_IF_handler(CMD_PREP_RX_BUF, &(p_hp->_buf), &(p_hp->_bufLen));

    printf("mcast_rx_main() to receive...\n");

    // Receive the message
    p_hp->_addrLen = sizeof(p_hp->_localAddr);
    p_hp->_nbytes = recvfrom(p_hp->_sock, p_hp->_buf, p_hp->_bufLen, 0, (struct sockaddr*)&(p_hp->_localAddr), &(p_hp->_addrLen));
    TEST_RECEIVED_SOCK(p_hp->_nbytes, SOCKET_ERROR, "recvfrom");
    
    printf("Received message len: %d\n", p_hp->_nbytes);
    p_as->_IF_handler(CMD_RX_DATA_READY, &(p_hp->_buf), &(p_hp->_bufLen));
    
    // Leave the multicast group
    setsockopt(p_hp->_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&(p_hp->_mcastReq), sizeof(p_hp->_mcastReq));

    // Close the socket
    closesocket(p_hp->_sock);
    return 0;
}

// Tx
int mcast_tx_main(struct a_sock *p_as) {
    struct heap_mcast_tx *p_hp = &(p_as->_heap._mcast_tx);
    int i;   
    // Create a socket for sending data
    p_hp->_sock = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_CREATED_SOCK(p_hp->_sock, INVALID_SOCKET, "sock");
    
    // Initialize the multicast address structure
    memset(&(p_hp->_mcastAddr), 0, sizeof(p_hp->_mcastAddr));
    p_hp->_mcastAddr.sin_family = AF_INET;
    p_hp->_mcastAddr.sin_addr.s_addr = inet_addr(p_as->_nic_info._ipv4._mcast_grp);
    p_hp->_mcastAddr.sin_port = htons(p_as->_nic_info._portNo);
    p_as->_IF_handler(CMD_PREP_TX_DATA, &(p_hp->_buf), &(p_hp->_bufLen));  // To let caller prepare tx data
    
    printf("mcast_tx_main() to send mcast message:");
    do{
        i += 1;
        // Send the message
        TRY_6_P_FUNC(sendto, p_hp->_sock, p_hp->_buf, p_hp->_bufLen, 0, (struct sockaddr*)&(p_hp->_mcastAddr), sizeof(p_hp->_mcastAddr), SOCKET_ERROR, "sendto");
        
        printf(".");
        
        Sleep(1000);  // sleep for 1000msec
    }while(i < 30);

    // Close the socket
    closesocket(p_hp->_sock);
    return 0;
}