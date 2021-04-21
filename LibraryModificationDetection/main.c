#include "main.h"

char poisoned_string[] = "the function has been modified\n";

int main(int argc, char** argv){
    printf("loading the library\n");
    void* handle = dlopen("./library.so", RTLD_NOW);
    int (*fcnptr_1)() = dlsym(handle, "test_1");
    printf("test function value: %d\n", fcnptr_1());
    printf("changing memory page protections\n");
    mprotect((void*)((unsigned long) fcnptr_1 - ((unsigned long) fcnptr_1 % getpagesize())),
        getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
    printf("trying to change function return value\n");
    *((char*)fcnptr_1 + 5) = 0;
    printf("test function value: %d\n", fcnptr_1());
    
    void (*fcnptr_2)() = dlsym(handle, "test_2");
    printf("testing library print function\n");
    fcnptr_2();
    printf("modifying library string function\n");
    mprotect((void*)((unsigned long) fcnptr_2 - ((unsigned long) fcnptr_2 % getpagesize())),
        getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
    //unsigned long long patch = 0x2e7b;
    //memcpy(fcnptr_2+7, &patch, 2);
    fcnptr_2();
    dlclose(handle);
}
