// tcp_client.c
#include <stdio.h>
#include <stdlib.h>
#include "../tcp/oldtcp.h"

int main(int argc, char**argv){
    if(argc != 3){
        printf("usage: %s server-ip my-ip\n",argv[0]);
        return -1;
    }
    return tcp_client(argv[1], argv[2]);
}