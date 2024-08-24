// To test "Pass buffer pointers" and use buffers in diferent layers
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../tcp/nic_info.h"

struct tcp_task tcpTask;
char rxBuf[32] = "THIS IS RX BUF";
int  rxBufLen = 32;
char txBuf[32] = "THIS IS TX BUF";
int  txBufLen = 32;
int on_prep_rx_buf(char**p_buf, int*p_buf_len){
    printf("on_prep_rx_buf() Hello!\n");
    *p_buf = &rxBuf[0];
    *p_buf_len = rxBufLen;
    printf("on_prep_rx_buf() %s, %d\n", *p_buf, *p_buf_len);
    return 0;
}
int on_rx_data_ready(char**p_buf, int*p_buf_len){
    printf("on_rx_data_ready() %s, %d\n", *p_buf, *p_buf_len);
    return 0;
}
int on_prep_tx_data(char**p_buf, int*p_buf_len){
    *p_buf = &txBuf[0];
    *p_buf_len = txBufLen;
    return 0;
}
int on_tx_data_done(char**p_buf, int*p_buf_len){
    printf("on_tx_data_done() %s, %d\n", *p_buf, *p_buf_len);
    return 0;
}
int(*vector_table[4])(char**,int*)={
    on_prep_rx_buf,
    on_rx_data_ready,
    on_prep_tx_data,
    on_tx_data_done
};
int tcp_handler(int cmd, char **p_buf, int *p_buf_len){
    printf("tcp_handler() %d\n", cmd);
    return vector_table[(0x0003 & (short)cmd)](p_buf, p_buf_len);
}
int main(void){
    printf("Hello world!\n");
    sprintf(tcpTask._ipv4_ip,"0.0.0.0");
    tcpTask._port_no = 0;
    tcpTask._handler = tcp_handler;
    char *buf;
    int   buf_len;
    tcpTask._handler(CMD_PREP_RX_BUF, &buf, &buf_len);
    char my_msg[]="Ha Ha...";
    int my_len = sizeof(my_msg);
    buf = memcpy(buf, my_msg, my_len);
    buf_len = my_len;
    tcpTask._handler(CMD_RX_DATA_READY, &buf, &buf_len);
    tcpTask._handler(CMD_PREP_TX_DATA, &buf, &buf_len);
    printf("main() tx data=%s, len=%d\n", buf, buf_len);
    tcpTask._handler(CMD_TX_DATA_DONE, &buf, &buf_len);
    return 0;
}