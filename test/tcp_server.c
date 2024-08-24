// tcp_server.c
#include <stdio.h>
#include <stdlib.h>
#include "../tcp/oldtcp.h"

int main(int argc, char**argv){
    if(argc != 2){
        printf("usage: %s my-ip\n",argv[0]);
        return -1;
    }
    return tcp_server(argv[1]);
}