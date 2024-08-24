#include <winsock2.h>
#include <ws2tcpip.h>
#include "../socket_tools/socket_info.h"

static int on_prep_rx_buf(char**, int*);
static int on_rx_data_ready(char**, int*);
static int on_prep_tx_data(char**, int*);
static int on_tx_data_done(char**, int*);
static int(*_vector_table[4])(char**,int*)={
    on_prep_rx_buf,
    on_rx_data_ready,
    on_prep_tx_data,
    on_tx_data_done
};
static int _handler(int cmd, char **p_buf, int *p_buf_len){
    return _vector_table[(0x0003 & (short)cmd)](p_buf, p_buf_len);
}

static WSADATA _wsaData;

static int destroy_a_sock(void){
    if(WSACleanup() != 0){
        printf("destroy_mcast_rx_sock() Failed:%d\n", WSAGetLastError());
        return -1;
    }
    return 0;
}
static int init_sock_sock(struct a_sock *p_as){
    if(WSAStartup(MAKEWORD(2,2), &_wsaData) != 0){
        printf("init_sock_sock() Failed:%d\n", WSAGetLastError());
        p_as->_IF_handler = NULL;
        return -1;
    }
    p_as->_IF_handler = _handler;
    return 0;
}