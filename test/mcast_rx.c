#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "../socket_tools/socket_IF.h"
#include "../socket_tools/mcast.h"
#include "peer_if.h"

struct mcast_task mcastTask;
char rxBuf[32];
static int on_prep_rx_buf(char**p_buf, int*p_buf_len){
    *p_buf = &rxBuf[0];
    *p_buf_len = sizeof(rxBuf);
    return 0;
}
static int on_rx_data_ready(char**p_buf, int*p_buf_len){
    struct peer_cmd *p_pcmd = (struct peer_cmd*)*p_buf;
    if(p_pcmd->_head._data_sz != 24){
        printf("on_rx_data_ready() data size error:%d !\n", p_pcmd->_head._data_sz);
        return -1;
    }else{
        struct peer_ipv4*p_pipv4 = (struct peer_ipv4*)p_pcmd->_data_buf;
        printf("on_rx_data_ready() received multicasted host at %s:%d\n", p_pipv4->_ipv4_addr, p_pipv4->_port_no);
    }
    return 0;
}
static int on_prep_tx_data(char**p_buf, int*p_buf_len){
    return 0;
}
static int on_tx_data_done(char**p_buf, int*p_buf_len){
    return 0;
}
int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s <mcast group(ip)> <mcast port>", argv[0]);
        return -1;
    }
    // struct socket_info *p_sI = init_a_sock();
    struct a_sock asck;
    memset(&asck, 0, sizeof(asck));
    int iret = init_sock_sock(&asck);
    if((iret != 0) || (asck._IF_handler == NULL)){
        printf("%s failed at init_a_sock()\n", argv[0]);
        return -1;
    }
    sprintf(asck._nic_info._ipv4._mcast_grp,"%s", argv[1]);
    asck._nic_info._portNo = atoi(argv[2]);
    mcast_rx_main(&asck);
    return destroy_a_sock();
}