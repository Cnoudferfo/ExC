#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "../socket_tools/socket_IF.h"
#include "../socket_tools/tcp.h"
#include "peer_if.h"

struct tcp_task tcpTask;
char rxBuf[32];
char txBuf[32];
int on_prep_rx_buf(char**p_buf, int*p_buf_len){
    *p_buf = &rxBuf[0];
    *p_buf_len = sizeof(rxBuf);
    return 0;
}
int on_rx_data_ready(char**p_buf, int*p_buf_len){
    // To catch end a server command
    struct peer_cmd *p_pcmd = (struct peer_cmd*)*p_buf;
    if(p_pcmd->_head._cmd == PEER_CMD_END_SERVER){
        return -1;
    }
    else{
        if(p_pcmd->_head._data_sz != 24){
            printf("on_rx_data_ready() data size error:%d !\n", p_pcmd->_head._data_sz);
            return -1;
        }else{
            struct peer_ipv4*p_pipv4 = (struct peer_ipv4*)p_pcmd->_data_buf;
            printf("on_rx_data_ready() received device register at %s:%d\n", p_pipv4->_ipv4_addr, p_pipv4->_port_no);
        }
    }
    return 0;
}
int on_prep_tx_data(char**p_buf, int*p_buf_len){
    struct peer_cmd *p_pcmd = (struct peer_cmd*)&txBuf[0];
    p_pcmd->_head._cmd = PEER_CMD_NULL;
    p_pcmd->_head._data_sz = 24;
    memset(p_pcmd->_data_buf, 0, p_pcmd->_head._data_sz);
    sprintf(p_pcmd->_data_buf, "Hello client!");
    *p_buf = &txBuf[0];
    *p_buf_len = sizeof(struct cmd_head) + p_pcmd->_head._data_sz;
    return 0;
}
int on_tx_data_done(char**p_buf, int*p_buf_len){
    // [todo] Do something
    return 0;
}
// Syntax: $exename host_ipv4_address host_port
int main(int argc, char **argv){
    if (argc != 3){
        printf("Usage: $exename host_ipv4_address host_port\n");
        return -1;
    }
    printf("%s on %s:%d\n",argv[0], argv[1], atoi(argv[2]));
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
    tcp_server(&asck, FLAG_FOREVER);
    printf("%s is to exit...\n", argv[0]);
    return destroy_a_sock();
}