#include "main.h"

int main(int argc, char** argv){
    printf("attempting to open library.so\n");
    void* handle = dlopen("./library.so", RTLD_NOW);
    if(handle == NULL){
        printf("Error opening library\n");
        return 1;
    }
    void(*fptr)() = dlsym(handle, "dynamic_function");
    fptr();
    dlclose(handle);
}
