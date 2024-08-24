// sim_tcp.c
// To use this single .exe to run clients and server in thread pool
#include <stdio.h>
#include <stdlib.h>
#include <synchapi.h>
#include "../threadpool/threadpool.h"
#include "../tcp/oldtcp.h"
#include "../a_queue/queue_state.h"

#define SERVER 0
#define CLIENT 1
#define INVALID -1
struct task_data{
        int  _task_type;      // 0:server, 1:client, -1:NULL
        int  _port;       // port number
        int(*_handler)(void*);// handler function
        char _my_ip[16];      // bind to this ip        
        char _opt_svr_ip[16]; // only used by clients
} ;
int client_handler(void*);
int server_handler(void*);
int handler(void*);

int main(void){
    struct task_data t_data[16] = {
        {SERVER, 27015, server_handler, "30.30.30.1", ""},            // 0
        {CLIENT, 27015, client_handler, "30.30.30.2", "30.30.30.1"},  // 1
        {CLIENT, 27015, client_handler, "30.30.30.3", "30.30.30.1"},  // 2
        {CLIENT, 27015, client_handler, "30.30.30.4", "30.30.30.1"},  // 3
        {CLIENT, 27015, client_handler, "30.30.30.2", "30.30.30.1"},  // 4
        {CLIENT, 27015, client_handler, "30.30.30.3", "30.30.30.1"},  // 5
        {CLIENT, 27015, client_handler, "30.30.30.4", "30.30.30.1"},  // 6
        {CLIENT, 27015, client_handler, "30.30.30.2", "30.30.30.1"},  // 7
        {CLIENT, 27015, client_handler, "30.30.30.3", "30.30.30.1"},  // 8
        {CLIENT, 27015, client_handler, "30.30.30.4", "30.30.30.1"},  // 9
        {CLIENT, 27015, client_handler, "30.30.30.2", "30.30.30.1"},  // 10
        {INVALID, 0,    NULL,           "",           ""}
    };
    int iret;

    iret = threadpool_init();
    for(int i=0; i<12; i++){
        if(t_data[i]._task_type != INVALID){
            threadpool_submit_task(i, (void*)&t_data[i], sizeof(t_data[i]), t_data[i]._handler);
            Sleep(500);
        }
    }
    while(a_queue_getQueueState() == STATE_OCPY_ONE){
        Sleep(100);
    }
    iret = threadpool_destroy();
}

int client_handler(void *data){
    struct task_data *p_t_data = (struct task_data*)data;
    return tcp_client(p_t_data->_opt_svr_ip, p_t_data->_my_ip);
}
int server_handler(void *data){
    struct task_data *p_t_data = (struct task_data*)data;
    return tcp_server(p_t_data->_my_ip);
}
int null_handler(void*data){
    printf("Shouldn't came here\n");
    return -1;
}
int handler(void*data){
    int(*vector[4])(void*)={
        server_handler,
        client_handler,
        null_handler,
        null_handler
    };
    struct task_data *pt=(struct task_data*)data;
    return vector[((unsigned int)(pt->_task_type) & 0x0003)](data);
}