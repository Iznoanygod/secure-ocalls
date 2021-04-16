#include "library.h"

__attribute__((constructor))
void construction(){
    printf("library successfully loaded\n");
}

void dynamic_function(){
    printf("this function was called dynamically\n");
}
