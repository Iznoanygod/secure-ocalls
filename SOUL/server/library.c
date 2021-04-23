#include "library.h"

__attribute__((constructor))
void construction(){
    printf("library successfully loaded\n");
}

void dynamic_function(){
    printf("this function was called dynamically\n");
}

int test_function(){
    return 1 ? 1 : 0;
}

int response(int challenge){
    srand(RANDOM_NUM + challenge);
    return rand();
}
