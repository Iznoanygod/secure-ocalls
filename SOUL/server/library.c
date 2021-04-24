#include "library.h"

__attribute__((constructor))
void construction(){
    printf("library successfully loaded\n");
}

int string_print_test(){
    char str1[] = "this function has not been modified\n";
    char str2[] = "this function has been modified\n";
    printf(str1);
    return 0;
}

int basic_return_test(){
    return 1 ? 1 : 0;
}



int response(int challenge){
    srand(RANDOM_NUM + challenge);
    return rand();
}
