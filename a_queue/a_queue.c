#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>      // For use uintptr_t type
#include "queue_state.h" // For calling modules to get queue state

///////////////////////////////////////////////////////////////////
// A ring queue
///////////////////////////////////////////////////////////////////
int iBufferSize;
#define TOP     0
#define BOTTOM  iBufferSize
void *pBuffer;
struct que_chunk{
    int    _serialNo;
    int    _data_size;
    int  (*_p_hd)(void*);
    struct que_chunk * _p_next;
    char   _data[];
};
#define TASK_SIZE(x) ((x)+sizeof(struct que_chunk))

// Queue indices
struct que_chunk *pQueHead;
struct que_chunk *pQueTail;
struct que_chunk *pLastQue;
int   iHeadPos;
int   iTailPos;
#define UPDATE_QUEUE_INDEX(x)  ((int)(uintptr_t)x)-((int)(uintptr_t)pBuffer)

#define IS_CROSS_NO 0
#define IS_CROSS_YES 1

// Queue state machine
int iQueState;

// Queue operation
#define OP_ADD      4001
#define OP_GET      5001

#define WBT(x)  printf("  Entered path_%d\n",  x)

// To add a task to the tail of queue
struct que_chunk * add_a_task_to_tail(void *p_data, int data_sz, int(*p_func)(void*)){
    struct que_chunk * rt;  // a task pointer
    void * p_buf;

    // point to the tail
    rt = pQueTail;
    // Copy task data
    rt->_data_size = data_sz;
    rt->_p_hd = p_func;
    p_buf = memcpy((void*)&(rt->_data[0]), p_data, rt->_data_size) + rt->_data_size;
    rt->_p_next = (struct que_chunk *)p_buf;
    // Shift the tail
    pQueTail = rt->_p_next;
    // Update the byte index of tail
    iTailPos = UPDATE_QUEUE_INDEX(pQueTail);
    // Update the pointer of the last task
    pLastQue = rt;

    return rt;
}

// Initialize queue indices back to empty state
void init_queue_indices(void){
    // Set queue head pointer and queue tail pointer
    pQueHead = (struct que_chunk*)pBuffer;
    pQueTail = pQueHead;

    // Set index number of queue head and tail
    iHeadPos = UPDATE_QUEUE_INDEX(pQueHead);
    iTailPos = UPDATE_QUEUE_INDEX(pQueHead);
}

// To get a task from the head of queue
struct que_chunk * get_a_task_from_head(void){
    struct que_chunk * rt;
    // Point to the current queue head
    rt = pQueHead;
    // Point the queue head to the task next to the current queue head
    pQueHead = rt->_p_next;
    // Update the byte index of head
    iHeadPos = UPDATE_QUEUE_INDEX(pQueHead);
    return rt;
}
void * queue_state_machine(int op, void *p_data, int data_size, int(*p_func)(void*)){
    struct que_chunk * rt = NULL;
    int chunk_sz = TASK_SIZE(data_size);
    
    switch( iQueState ){
        case STATE_EMPTY:
            if(op == OP_GET){
                printf("queue_state_machine(), A, Empty queue, nothing to get.\n");
            }
            else if(op == OP_ADD){
                if((iTailPos + chunk_sz) > BOTTOM){
                    printf("queue_state_machine(), B, Insufficient free space, try later\n");
                }
                else{   /* CHECKED */
                    rt = add_a_task_to_tail(p_data, data_size, p_func);
                    iQueState = STATE_OCPY_ONE;
                    printf("queue_state_machine(), C, add a task, head=%d, tail=%d\n", iHeadPos, iTailPos);
                }
            }
            else{
                printf("queue_state_machine(), D, Fatal logic error, invalid op value=%d\n", op);
            }
            break;
        case STATE_OCPY_ONE:
            if(op == OP_GET){
                rt = get_a_task_from_head();
                printf("queue_state_machine(), E, get a task, head=%d, tail=%d\n", iHeadPos, iTailPos);
                if(iTailPos == iHeadPos){
                    init_queue_indices();
                    iQueState = STATE_EMPTY;
                    printf("queue_state_machine(), F, init_queue_indices, head=%d, tail=%d\n", iHeadPos, iTailPos);
                }
            }
            else if(op == OP_ADD){
                int theLimit = (iHeadPos < iTailPos? BOTTOM : iHeadPos);
                if((iTailPos + chunk_sz) > theLimit){
                    if((iTailPos > iHeadPos)&&(iHeadPos > TOP)&&(chunk_sz < iHeadPos)){
                        // Cross boundary
                        pQueTail = (struct que_chunk*)pBuffer;
                        pLastQue->_p_next = pQueTail;
                        rt = add_a_task_to_tail(p_data, data_size, p_func);
                        printf("queue_state_machine(), G, add a task, head=%d, tail=%d\n", iHeadPos, iTailPos);
                    }
                    else{
                        printf("queue_state_machine(), H, Insufficient free space, try later\n");
                    }
                }
                else{
                    rt = add_a_task_to_tail(p_data, data_size, p_func);
                    printf("queue_state_machine(), I, add a task, head=%d, tail=%d\n", iHeadPos, iTailPos);
                }
            }
            else{
                printf("queue_state_machine(), J, Fatal logic error, invalid op value=%d\n", op);
            }
            break;
        default:
            printf("queue_state_machine(), K\n");
            break;
    }
    return rt;
}

// Interface function
int a_queue_init(int buf_sz){
    // Initialize a queue
    iBufferSize = buf_sz;
    
    // Allocate buffer
    pBuffer = malloc(iBufferSize);
    if(pBuffer==NULL){
        printf("malloc failed\n");
        return -1;
    }
    else{
        printf("init_my_little_queue() malloc(%d), pBuffer=%d\n", iBufferSize, (uintptr_t)pBuffer);
    }

    init_queue_indices(); // Initialize queue index
    
    iQueState = STATE_EMPTY;    // Set queue state to EMPTY
    return 0;
}
// Interface function
int a_queue_cleanup(void){
    free(pBuffer);
    printf("free_my_little_queue() at iHeadPos=%d, iTailPos=%d\n", iHeadPos, iTailPos);
}
// Interface function
int a_queue_addTask(int s_no, void *p_data, int data_size, int(*p_func)(void*)){
    
    if(NULL == queue_state_machine(OP_ADD, p_data, data_size, p_func)){
        return -1;  // Return failed
    }
    else{
        return 0;   // Return successfully
    }
}
// Interface function
// Returned value : pointer to data handler function
int(*a_queue_takeTask(void**pp_data, int *data_sz))(void*){
    struct que_chunk *rt = NULL;
    
    rt = (struct que_chunk*)queue_state_machine(OP_GET, NULL, 0, NULL);
    if(rt != NULL){
        int(*p_func)(void*);
        // int ret;
        // ret = rt->_p_hd((void*)rt->_data);  // Call data handler function
        // return 0;
        // printf("rt %d, %d\n",rt->_data_size, rt->_serialNo);
        *data_sz = rt->_data_size;
        p_func = rt->_p_hd;
        *pp_data = rt->_data;
        return p_func;
    }
    else{
        return NULL;
    }
}
// Interface function
int a_queue_getQueueState(void){
    return iQueState;
}