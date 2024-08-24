#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "../socket_tools/socket_IF.h"
#include "../socket_tools/tcp.h"
#include "peer_if.h"

int to_end_server;

struct tcp_task tcpTask;
char rxBuf[32];
char txBuf[32];
int on_prep_rx_buf(char**p_buf, int*p_buf_len){
    *p_buf = &rxBuf[0];
    *p_buf_len = sizeof(rxBuf);
    return 0;
}
int on_rx_data_ready(char**p_buf, int*p_buf_len){
    // [todo] Do something
    // For test, , just to print the rx data
    printf("client on_rx_data_ready()  rx_msg=%s, len=%d\n", ((struct peer_cmd*)*p_buf)->_data_buf, *p_buf_len);
    return 0;
}
int on_prep_tx_data(char**p_buf, int*p_buf_len){
    struct peer_cmd *p_pcmd = (struct peer_cmd*)&txBuf[0];
    if(to_end_server == 1){
        p_pcmd->_head._cmd = PEER_CMD_END_SERVER;
        p_pcmd->_head._data_sz = 24;
        memset(p_pcmd->_data_buf, 0, p_pcmd->_head._data_sz);
        sprintf(p_pcmd->_data_buf,"Hello server! Time to end.");
    }
    else{
        p_pcmd->_head._cmd = PEER_CMD_REG_DEVICE;
        p_pcmd->_head._data_sz = 24;
        struct peer_ipv4 *p_pipv4=(struct peer_ipv4*)p_pcmd->_data_buf;
        memset(p_pipv4, 0, p_pcmd->_head._data_sz);
        p_pipv4->_port_no = 27015;
        sprintf(p_pipv4->_ipv4_addr,"127.0.0.1");
    }
    *p_buf = &txBuf[0];
    *p_buf_len = sizeof(struct cmd_head) + p_pcmd->_head._data_sz;
    return 0;
}
int on_tx_data_done(char**p_buf, int*p_buf_len){
    // [todo] Do something
    return 0;
}
int main(int argc, char**argv){
    if (argc < 3){
        printf("Usage: %s host_ipv4_address host_port [1:to end server]\n", argv[0]);
        return -1;
    }
    if((argc==4) && (atoi(argv[3])==1)){
        to_end_server = 1;
    }
    else{
        to_end_server = 0;
    }
    printf("%s to %s:%d\n", argv[0], argv[1], atoi(argv[2]));
    // struct socket_info *p_sI = init_a_sock();
    struct a_sock asck;
    memset(&asck, 0, sizeof(asck));
    int iret = init_sock_sock(&asck);
    if((iret != 0) || (asck._IF_handler == NULL)){
        printf("%s failed at init_sock_sock()\n", argv[0]);
        return -1;
    }
    sprintf(asck._nic_info._ipv4._ip_addr,"%s", argv[1]);
    asck._nic_info._portNo = atoi(argv[2]);
    tcp_client(&asck);
    return destroy_a_sock();
}