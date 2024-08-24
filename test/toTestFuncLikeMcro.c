#include <stdio.h>
#include <stdlib.h>

int lastError;
#define ERROR_NO_ERROR 0
#define ERROR_GT_TWO -1001
#define ERROR_GT_THREE -2001
#define ERROR_RET_ERR -1
#define EXIT_W_ERR(fname, gle) ({ \
    printf("%s() Failed:%d\n",fname, gle()); \
    return -1; \
})
#define TRY_2_PAR(func, p1, p2, fname, err) ({ \
    if(func(p1, p2) == err){ \
        EXIT_W_ERR(fname, getLastError); \
    } \
})

#define TRY_3_PAR(func, p1, p2, p3, fname, err) ({ \
    if(func(p1, p2, p3) == err){ \
        EXIT_W_ERR(fname, getLastError); \
    } \
})

int add_two(int *a, int b){
    if((*a+b) <= 2){
        return 0;
    }
    else {
        lastError = ERROR_GT_TWO;
        return ERROR_RET_ERR;
    }
}
int add_three(int *a, int b, int c){    
    if((*a+b+c) <= 3){
        return 0;
    }
    else{
        lastError = ERROR_GT_THREE;
        return ERROR_RET_ERR;
    }
}
int getLastError(void){
    return lastError;
}
int main(void){
    lastError = ERROR_NO_ERROR;
    short s=1;
    int   i=1;
    int   j=0;
    TRY_2_PAR(add_two, (int*)&s, sizeof(s), "add_two", ERROR_RET_ERR);
    TRY_3_PAR(add_three, (int*)&s, i, j, "add_three", ERROR_RET_ERR);
    return 0;
}