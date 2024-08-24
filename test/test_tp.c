// A sample program to test a thread pool with ring queue
// How to build : gcc -o test_tp test_tp.c ../threadpool/threadpool.c ../a_queue/a_queue.c -pthread
#include <stdio.h>
#include <stdlib.h>
#include <synchapi.h>
#include "../threadpool/threadpool.h"
#include "../a_queue/queue_state.h"

///////////////////////////////////////////////////////////////////
// A little tester
///////////////////////////////////////////////////////////////////
//    A sub data for testing task add a sub task
struct sub_data {
    int _a;
    int _b;
};
int sub_handler(void*arg){
    struct sub_data data = *((struct sub_data*)arg);
    printf("sub_handler() : Hi there! _a=%d, _b=%d\n", data._a, data._b);
    return 0;
}
//    A data struct for test
struct test_data {
    /* data */
    int _accu;  // Summation value
    int _incr;  // Incremental value
    int _iter;  // Number of iterations
};
//    A test implementation of a task handler
int test_handler(void *arg){
    struct test_data data = *((struct test_data*)arg);
    int i;
    for(i=0;i<data._iter;i++){
        data._accu += data._incr;
    }
    printf("test_handler(): accu=%d, incr=%d, iter=%d\n", data._accu, data._incr, data._iter);
    if(data._accu > 10){
        struct sub_data s_data={._a=2, ._b=4};
        int iret;
        do{
            // To submit a sub task from a task
            iret = threadpool_submit_task(0,(void*)&s_data, sizeof(s_data), sub_handler);
            Sleep(200);
        }while(iret != 0);
    }
    return 0;
}

//    A tester body to test thread pool
int to_test_thread_pool(void){
    int iRet;
    int i;

    if(threadpool_init(8) < 0){   // Init thread pool
        printf("to_test_thread_pool(){threadpool_init() Failed}\n");
        return -1;
    }
    srand(3768);
    for(i=0;i<20;i++){
        struct test_data t_data={
            ._accu = rand() % 5,
            ._incr = rand() % 4 + 1,
            ._iter = rand() % 10 + 1
        };
        do{
            // In case task queue was full, wait a while and try again
            iRet = threadpool_submit_task(i,(void*)&t_data, sizeof(t_data), test_handler);
            Sleep((rand() % 6)<<4);
        }while(iRet!=0);
    }
    while(a_queue_getQueueState()==STATE_OCPY_ONE){
        Sleep(100);
    }
    return threadpool_destroy();    // Cleanup thread pool
}

int main(void){
    return to_test_thread_pool();
}