#include "App.h"

sgx_enclave_id_t global_eid = 0;
void* handle;

int simpleRead(int fd, char* buffer, int maxRead){
    int size;
    if(maxRead == -1)
        size = lseek(fd, 0, SEEK_END);
    else
        size = maxRead;
    lseek(fd, 0, SEEK_SET);
    int readin = 0;
    while(1){
        int status = read(fd, buffer+readin, size-readin);
        if(status == -1){
            return 0;
        }
        readin += status;
        if(readin == size || status == 0)
            break;
    }
    return readin;
}

int simpleWrite(int fd, char* buffer, int maxWrite){
    int size;
    if(maxWrite == -1)
        size = strlen(buffer);
    else
        size = maxWrite;
    int writeout = 0;
    while(1){
        int status = write(fd, buffer+writeout, size-writeout);
        if(status == -1){
            return 0;
        }
        writeout += status;
        if(writeout == size || status == 0)
            break;
    }
    return writeout;
}

int ocall_runfcn(void* fcnptr){
    int(*fcn)(void) = fcnptr;
    int status = fcn();
    return status;
}

void ocall_print(const char* str){
    printf("%s", str);
}

void* ocall_getSym(const char* str) {
    void* fcnptr = dlsym(handle, str);
    return fcnptr;
}

int initialize_enclave(char* enclave_file) {
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = sgx_create_enclave(enclave_file, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if(ret != SGX_SUCCESS) {
        printf("Enclave initialization error %d\n", ret);
        return -1;
    }
    return 0;
}

int SGX_CDECL main (int argc, char** argv) {
    if(argc < 3){
        printf("usage: App server_ip port\n");
        return 0;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);

    printf("Connecting to library server\n");
    struct sockaddr_in serv_addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if(sock < 0){
        printf("Error connecting to server\n");
        return 1;
    }
    printf("Connected to server...\n");
    
    printf("Retrieving Library File\n");
    simpleWrite(sock, "10:getLibrary", 13);
    printf("Sent command getLibrary\n");
    char buffer[512];
    int inputread = 0;
    while(1){
        read(sock, buffer + inputread, 1);
        if(buffer[inputread] == ':'){
            buffer[inputread] = '\0';
            break;
        }
        inputread++;
    }
    int size = atoi(buffer);
    char* libraryso = malloc(sizeof(char) * size);
    simpleRead(sock, libraryso, size);
    int libraryFD = open("library.so", O_RDWR | O_CREAT, S_IRWXU);
    simpleWrite(libraryFD, libraryso, size);
    close(libraryFD);
    free(libraryso);
    printf("Retrieved Library File\n");

    printf("Getting Hash File\n");
    simpleWrite(sock, "7:getHash", 9);
    inputread = 0;
    while(1){
        read(sock, buffer + inputread, 1);
        if(buffer[inputread] == ':'){
            buffer[inputread] = '\0';
            break;
        }
        inputread++;
    }
    size = atoi(buffer);
    char *hashDump = malloc(size);
    simpleRead(sock, hashDump, size);
    int hashfd = open("hash.dmp", O_RDWR | O_CREAT, S_IRWXU);
    simpleWrite(hashfd, hashDump, size);
    close(hashfd);
    free(hashDump);
    
    printf("Got Hash File\n");

    // Mock challenge response, this would normally be done by the enclave
    // requires remote attestation first and a secure connection from the enclave
    // the remote enclave
    
    handle = dlopen("./library.so", RTLD_NOW);

    printf("Checking challenge-response\n");
    int(*response)(int) = dlsym(handle, "response");
    simpleWrite(sock, "11:getResponse", 14);
    int challenge;
    simpleRead(sock, (char*)(&challenge), sizeof(int));
    int challenge_response = response(challenge);
    simpleWrite(sock, (char*)(&challenge_response), sizeof(int));
    char c_response[8];
    simpleRead(sock, c_response, 7);
    c_response[7] = '\0';
    if(!strcmp(c_response, "failure")){
        printf("Challenge response did not match!\n");
        return -1;
    }
    printf("Challenge response matched\n");

    // Now we can initialize the enclave, load in the function symbols and hash values
    // then call the main enclave ECall which verifies and runs all our library functions

    printf("Initializing Enclave\n");
    if(initialize_enclave("enclave.signed.so") < 0) {
        printf("Couldn't initialize enclave\n");
        return -1;
    }
    printf("Initialized Enclave\n");

    printf("Registering all library functions\n");
    
    //void*(*string_print_test)() = dlsym(handle, "string_print_test");
    //enclave_add_function(global_eid, (unsigned long long)string_print_test, 176, "3dc237adbd042aeda6617b4c23a73f3d6e57f56b");
    FILE* hash_file = fopen("hash.dmp", "r");
    char func_name[512];
    int func_size;
    char func_hash[41];
    while(fscanf(hash_file, "%s %d %s\n", func_name, &func_size, func_hash) != EOF){
        void* fcnptr = dlsym(handle, func_name);
        enclave_add_function(global_eid, func_name, fcnptr, func_size, func_hash);
        printf("Registered %s, hash %s\n", func_name, func_hash);
    }
    printf("Registered all functions\n");

    simpleWrite(sock, "10:disconnect", 13);
    close(sock);
    // Calling enclave, no modification of library functions 
    enclave_run(global_eid);

    //Modifying library functions
    printf("Modifying Library functions\n");
    void*string_print_test = dlsym(handle, "string_print_test");
    void*basic_return_test = dlsym(handle, "basic_return_test");
    
    printf("Modifying library string function\n");
    mprotect((void*)((unsigned long) string_print_test - ((unsigned long) string_print_test % getpagesize())),
        getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
    unsigned long long patch = 0xa0;
    printf("Trying to patch string\n");
    memcpy(((char*)string_print_test+133), &patch, 1);

    printf("Modifying basic_return_test\n");
    mprotect((void*)((unsigned long) basic_return_test - ((unsigned long) basic_return_test % getpagesize())),
        getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
    printf("Trying to change function return value\n");
    *((char*)basic_return_test + 5) = 0;

    // Calling enclave, modified library
    enclave_run(global_eid);

    dlclose(handle);
    //remove("./library.so");
    //remove("./hash.dmp");

}
