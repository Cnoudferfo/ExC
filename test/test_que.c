#include    <stdio.h>
#include    <stdlib.h>
#include    <memory.h>
#include    "a_queue.h"

// Prepare two data bodies for test
struct st_type_one{    // A task carried type one data has 32byte size
    int _v11;
    int _v12;
} data_one = {._v11=1, ._v12=2};
struct st_type_two{    // A task carried type two data has 36byte size
    int _v21;
    int _v22;
    int _v23;
} data_two = {._v21=3, ._v22=4, ._v23=5};

#define ADD_ONE 0
#define ADD_TWO 1
#define GET_ANY 2

// Data handler (call back) function
int do_data_one(void*p_src){
    struct st_type_one *p_dest;
    
    p_dest = malloc(sizeof(struct st_type_one));
    if(p_dest == NULL){
        return -1;
    }
    p_dest = (struct st_type_one*)memcpy((void*)p_dest, p_src, sizeof(struct st_type_one));
    printf("do_data_one() v11=%d, v12=%d\n", p_dest->_v11, p_dest->_v12);
    free(p_dest);
    return 0;
}

// Data handler (call back) function
int do_data_two(void*p_src){
    struct st_type_two *p_dest;
    
    p_dest = malloc(sizeof(struct st_type_two));
    if(p_dest == NULL){
        return -1;
    }
    p_dest = (struct st_type_two*)memcpy((void*)p_dest, p_src, sizeof(struct st_type_two));
    printf("do_data_two() v21=%d, v22=%d, v23=%d\n", p_dest->_v21, p_dest->_v22, p_dest->_v23);
    free(p_dest);
    return 0;
}

// To deal with a task carring type one data
void *add_data_one(int v){
    int iRet;

    data_one._v11 = ADD_ONE;
    data_one._v12 = 1001;
    iRet = a_queue_addTask(0, (void*)&data_one, sizeof(data_one), do_data_one);
    return &data_one;
}
// To deal with a task carring type two data
void *add_data_two(int v){
    int iRet;

    data_two._v21 = ADD_TWO;
    data_two._v22 = 2001;
    data_two._v23 = 20001;
    iRet = a_queue_addTask(0, (void*)&data_two, sizeof(data_two), do_data_two);
    return &data_two;
}
// To deal with a task carring type two data
void *get_data(int v){
    int(*p_func)(void*);
    void *data;
    int sz;
        
    p_func = a_queue_takeTask(&data,&sz);
    if((p_func != NULL) && (data != NULL)){
        int ret = p_func(data);  // Call data handler
    }
    
    return NULL;
}

int test_my_little_queue(void){
    int iRet;
    struct que_chunk *src, *dest;
    struct st_type_one *ptr;
    struct st_type_two *ptr2;
    int i;
#define TEST_SEG_SIZE 10
    int testSeq[TEST_SEG_SIZE]={
        // ADD_ONE,
        // ADD_ONE,
        // GET_ANY,
        // GET_ANY,
        // GET_ANY,
        // ADD_ONE,
        // ADD_ONE,
        // GET_ANY,
        // GET_ANY,
        // ADD_ONE

        // ADD_ONE,
        // ADD_ONE,
        // ADD_ONE,
        // ADD_ONE,
        // ADD_ONE,
        // ADD_ONE,
        // GET_ANY,
        // GET_ANY,
        // ADD_ONE,
        // ADD_ONE

        ADD_ONE, // 1
        GET_ANY, // 2
        ADD_TWO, // 3
        GET_ANY, // 4
        ADD_ONE, // 5
        GET_ANY, // 6
        ADD_TWO, // 7
        GET_ANY, // 8
        GET_ANY, 
        ADD_ONE  
    };

    void *(*prepArray[3])(int)  = { add_data_one, add_data_two, get_data };

    iRet = a_queue_init(128);

    for(i=0; i<TEST_SEG_SIZE; i++){
        printf("(%d) %s : ", i, (testSeq[i]) == 0 ? "ADD_ONE" : (testSeq[i] == 1 ? "ADD_TWO" : "GET_ANY"));
        prepArray[testSeq[i]](i);
    }    

    // Free buffer
    iRet = a_queue_cleanup();
    return 0;
}
int main(int argc, void *argv){
    return test_my_little_queue();
}