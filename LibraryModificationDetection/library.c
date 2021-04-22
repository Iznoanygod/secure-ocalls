#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

char test_1_digest[20];
char test_2_digest[20];

int test_1() {
    return 1 ? 1 : 0;
}

void test_2(){
    char str[] = "this function has not been modified\n";
    char poisoned_string[] = "this function has been modified\n";
    printf(str);
}

int check_modification(){
    char test_1_hash[20];
    char test_2_hash[20];
    SHA1((char*)test_1, 0xb, test_1_hash);
    SHA1((char*)test_2, 0xaa, test_2_hash);
    return memcmp(test_1_hash, test_1_digest, 20) & memcmp(test_2_hash, test_2_digest, 20);
}

__attribute__((constructor))
void constructor(){
    printf("library has been loaded\n");
    SHA1((char*)test_1, 0xb, test_1_digest);
    SHA1((char*)test_2, 0xb, test_2_digest);
}

