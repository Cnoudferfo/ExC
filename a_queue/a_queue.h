// ring_queue.h
// To declare interface functions of ring_queue.c
int a_queue_init(int);              // To initialize the queue
int a_queue_cleanup(void);             // To destroy the queue
int a_queue_addTask(int,void*,int,int(*)(void*));    // To add a task to queue
int(*a_queue_takeTask(void**,int*))(void*); // To get a task from queue
int a_queue_getQueueState(void);          // To get queue state
#define STATE_EMPTY     1001
#define STATE_OCPY_ONE  2001