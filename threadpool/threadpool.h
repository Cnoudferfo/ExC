// threadpool.h 

int threadpool_init(int);
int threadpool_submit_task(int, void*, int, int(*)(void*));
int a_queue_getQueueState(void);
int threadpool_destroy(void);