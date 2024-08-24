#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "../tcp/nic_info.h"
#include "../tcp/tcp.h"

struct tcp_task tcpTask;
char rxBuf[32];
char txBuf[32];
int on_prep_rx_buf(char**p_buf, int*p_buf_len){
    *p_buf = &rxBuf[0];
    *p_buf_len = sizeof(rxBuf);
    printf("server on_prep_rx_buf() rxBuf was assigned.\n");
    return 0;
}
int on_rx_data_ready(char**p_buf, int*p_buf_len){
    // [todo] Do something
    // For test, just to print the rx data
    printf("server on_rx_data_ready()  rx_msg=%s, len=%d\n", *p_buf, *p_buf_len);
    return 0;
}
int on_prep_tx_data(char**p_buf, int*p_buf_len){
    *p_buf = &txBuf[0];
    sprintf(txBuf,"Hello, client");
    *p_buf_len = strlen(txBuf);
    printf("server on_prep_tx_data(), to send %s, len=%d\n", *p_buf, *p_buf_len);
    return 0;
}
int on_tx_data_done(char**p_buf, int*p_buf_len){
    // [todo] Do something
    return 0;
}
int(*vector_table[4])(char**,int*)={
    on_prep_rx_buf,
    on_rx_data_ready,
    on_prep_tx_data,
    on_tx_data_done
};
int tcp_handler(int cmd, char **p_buf, int *p_buf_len){
    return vector_table[(0x0003 & (short)cmd)](p_buf, p_buf_len);
}
// Syntax: $exename host_ipv4_address host_port
int main(int argc, char **argv){
    if (argc != 3){
        printf("Usage: $exename host_ipv4_address host_port\n");
        return -1;
    }
    printf("%s on %s:%d\n",argv[0], argv[1], atoi(argv[2]));
    sprintf(tcpTask._ipv4_ip,"%s", argv[1]);
    tcpTask._port_no = atoi(argv[2]);
    tcpTask._handler = tcp_handler;
    int iret;
    iret = tcp_init();
    iret = tcp_server(&tcpTask);
    iret = tcp_destroy();
    return iret;
}