#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <synchapi.h>
#include <memory.h>
#include <stdint.h>      // For use uintptr_t type
#include "../a_queue/a_queue.h"     // A self-made queue module
#include "../a_queue/queue_state.h"

// Thread pool
//   A thread pool element
struct  a_thread  {
    int         _serial_no;      // Serial No of this thread
    pthread_t   _thread_id;      // Thread id
};
struct  a_thread  *thread_pool;    // Thread pool
int iThreadPoolSz;

// Abort the whole tread pool flag
#define ALL_ABORT_NO    0
#define ALL_ABORT_YES   1
int iAllAbort;

pthread_mutex_t threadMutex;   // Critical section lock
pthread_cond_t  threadCond;    // Thread condition variable

// Function body of each thread
void    *thread_function(void *arg){
    int mySerialNo = ((struct a_thread*)arg)->_serial_no;   // Serial number of this thread
    int i;
    int ret;

    printf("NO.%d thread was created\n", mySerialNo);
    while( ALL_ABORT_NO == iAllAbort ){         // Start the main wile loop
        
        ret = pthread_mutex_lock(&threadMutex); // Enter critical section to get a task
        while(STATE_EMPTY == a_queue_getQueueState()){               // Wait for a task to do
            ret = pthread_cond_wait( &threadCond, &threadMutex );    // Wait until a task was added into the queue
            if( ALL_ABORT_YES==iAllAbort ){                          // Break to abort
                break;
            }
        }
        if( ALL_ABORT_YES==iAllAbort ){              // Break to abort
            pthread_mutex_unlock( &threadMutex );    // Unlock the mutex manually
            break;
        }
        
        // To take a task from the head of the queue
        void *src = NULL;
        int   size;
        int(*p_func)(void*) = a_queue_takeTask(&src, &size);    // To get a task from queue
                                                                // p_func : points to the task hander funciton
                                                                // src : points to task data
                                                                // size : size of task data
        ret = pthread_mutex_unlock( &threadMutex ); // Leave critical section since a task has been got
        if((p_func != NULL) && (src != NULL)){
            void *dest = malloc(size);              // 
            if(dest != NULL){
                dest = memcpy(dest, src, size);
                int ret = p_func(dest);             // To call a data handler which is the main body of the task
                free(dest);
            }
        }
    }
    printf("No.%d thread to exit\n", mySerialNo);
    pthread_exit(NULL);
}

// Get the size of thread pool
int threadpool_get_thread_pool_size(void){
    return  iThreadPoolSz;
}

// Initialize the thread pool
int threadpool_init(int no_of_thrds){
    int ret;
    int i;

    // To allocate memory heap for thread pool
    thread_pool = malloc(sizeof(struct a_thread) * no_of_thrds);
    if(thread_pool == NULL){
        printf("thread_pool_init(){malloc() Failed}\n");
        return -1;
    }
    iThreadPoolSz = no_of_thrds;

    ret = a_queue_init((36<<4));    // Initialize a queue
    if(ret==-1){
        printf("thread_pool_init(){init_a_queue() Failed}\n");
        return -1;
    }

    iAllAbort = ALL_ABORT_NO;                       // Set all abort flag to NO
    
    // Initialize mutex lock & conditional variable
    ret = pthread_mutex_init(&threadMutex, NULL); // Init thread mutex
    printf("pthread_mutex_init()=%d\n", ret);
    ret = pthread_cond_init(&threadCond, NULL);   // Init thread cond
    printf("pthread_cond_init()=%d\n", ret);

    // Start the thread pool
    for(i=0;i<iThreadPoolSz;i++){
        thread_pool[i]._serial_no = i;  // A thread's serial number is i
        ret = pthread_create(&(thread_pool[i]._thread_id), NULL, thread_function, (void*)(thread_pool+i));
    }

    return 0;
}

// Clean up the thread pool
int threadpool_destroy(void){
    int ret;
    int i;

    printf("Waiting all thread to terminate...\n");

    // Set the all abort flag
    ret = pthread_mutex_lock(&threadMutex);
    iAllAbort = ALL_ABORT_YES;
    ret = pthread_cond_broadcast(&threadCond);
    ret = pthread_mutex_unlock(&threadMutex);

    // Wait for all thread to terminate
    for(i=0;i<iThreadPoolSz;i++){
        ret = pthread_join(thread_pool[i]._thread_id, NULL);
    }

    // Cleanup the resource
    ret = pthread_mutex_destroy(&threadMutex);
    printf("pthread_mutex_destroy()=%d\n", ret);
    ret = pthread_cond_destroy(&threadCond);
    printf("pthread_cond_destroy()=%d\n",ret);

    ret = a_queue_cleanup();   // Destroy a queue

    free(thread_pool);      // To free memory heap of thread pool

    return 0;
}

// To submit a task
int threadpool_submit_task(int serialNo, void *data, int dataSz, int (*handler)(void*)){
    int ret;
        
    pthread_mutex_lock(&threadMutex);     // Enter critical section
    ret = a_queue_addTask(serialNo, data, dataSz, handler);  // To add a task to queue
    pthread_mutex_unlock(&threadMutex);           // Leave critical section
    pthread_cond_signal(&threadCond);             // Signaling to threads
    return ret;   // Return 0 means task successfully submitted
}
