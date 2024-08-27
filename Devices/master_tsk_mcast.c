// master_tsk_mcast.c
//  A task of master devices's multicast process
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "../socket_tools/socket_IF.h"
#include "../socket_tools/mcast.h"
#include "interface.h"

static char txBuf[32];  // A local tx buffer
// Handlers on multicast requests
//  null handler
static int on_prep_rx_buf(char**p_buf, int*p_buf_len){
    return 0;
}
//  null handler
static int on_rx_data_ready(char**p_buf, int*p_buf_len){
    return 0;
}
//  only process "prepare tx data"
static int on_prep_tx_data(char**p_buf, int*p_buf_len){
    struct peer_cmd *p_pcmd = (struct peer_cmd*)&txBuf[0];
    p_pcmd->_head._cmd = PEER_CMD_HOST_MCAST;
    p_pcmd->_head._data_sz = 24;
    struct peer_ipv4 *p_pipv4=(struct peer_ipv4*)p_pcmd->_data_buf;
    memset(p_pipv4, 0, p_pcmd->_head._data_sz);
    p_pipv4->_port_no = 27015;
    sprintf(p_pipv4->_ipv4_addr, "127.0.0.1");
    *p_buf = &txBuf[0];
    *p_buf_len = sizeof(struct cmd_head) + p_pcmd->_head._data_sz;
    return 0;
}
//  null handler
static int on_tx_data_done(char**p_buf, int*p_buf_len){
    return 0;
}
// main body of master device's mcast task

int master_tsk_mcast_main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s <mcast group(ip)> <mcast port>\n", argv[0]);
        return -1;
        
    }
    struct a_sock asck;
    memset(&asck, 0, sizeof(asck));
    int iret = init_sock_sock(&asck);
    if((iret != 0) || (asck._IF_handler == NULL)){
        printf("%s failed at init_sock_sock()\n", argv[0]);
        return -1;
    }
    sprintf(asck._nic_info._ipv4._mcast_grp, "%s", argv[1]);
    asck._nic_info._portNo = atoi(argv[2]);
    mcast_tx_main(&asck);
    return destroy_a_sock();
}