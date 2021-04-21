#include "client.h"

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

int main(int argc, char** argv){
    if(argc < 3){
        printf("usage: client server_ip port\n");
        return 0;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);

    printf("Connecting to server...\n");
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
    int readin = 0;
    simpleRead(sock, libraryso, size);
    close(sock);
    int libraryFD = open("library.so", O_RDWR | O_CREAT, S_IRWXU);
    simpleWrite(libraryFD, libraryso, size);
    close(libraryFD);
    free(libraryso);
    printf("Retrieved Library File\n");
    printf("Loading Library File\n");
    void* handle = dlopen("./library.so", RTLD_NOW);
    if(handle == NULL){
        printf("Error opening library\n");
        return 1;
    }
    void(*fptr)() = dlsym(handle, "dynamic_function");
    fptr();
    dlclose(handle);
    remove("./library.so");
}
