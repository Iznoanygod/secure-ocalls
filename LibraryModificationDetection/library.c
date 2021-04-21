#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

__attribute__((constructor))
void constructor(){
    printf("library has been loaded\n");
}

int test_1() {
    return 1 ? 1 : 0;
}

void test_2(){
    char str[] = "this function has not been modified\n";
    char poisoned_string[] = "this function has been modified\n";
    printf(str);
}

