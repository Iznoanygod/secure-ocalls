#include "server.h"

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

int main(int argc, char** argv) {
    if(argc < 2){
        printf("usage: server port\n");
        return 1;
    }
    int port = atoi(argv[1]);
    
    printf("Attempting to bind to socket...\n");
    int opt = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFD == 0){
        printf("Server File Descriptor Error\n");
        return 1;
    }
    int sockopt = setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if(sockopt > 0) {
        printf("Failed to setsockopt\n");
        return 1;
    }
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(port);
    int bindret = bind(serverFD, (struct sockaddr*)&address, sizeof(address));
    if(bindret < 0){
        printf("Failed to bind to socket\n");
        return 1;
    }
    printf("Bound to socket...\n");
    
    if (listen(serverFD, 3) < 0) 
    { 
        printf("Failed to listen.\n"); 
        return 1;
    } 
    while(1){
        printf("Waiting for clients\n");
        int sock = accept(serverFD, (struct sockaddr*) &address, (socklen_t*)&addrlen);
        if(sock < 0){
            printf("Error accepting client\n");
            return 0;
        }
        printf("Accepted client...\n");
        
        int libraryFD = open("library.so", O_RDONLY);
        int size = lseek(libraryFD, 0, SEEK_END);
        lseek(libraryFD, 0, SEEK_SET);

        char* libraryso = malloc(sizeof(char)*size);
        simpleRead(libraryFD, libraryso, size);
        close(libraryFD);
        char message[512];
        sprintf(message, "%d:", size);
        simpleWrite(sock, message, strlen(message));
        simpleWrite(sock, libraryso, size);
        free(libraryso);
        close(sock);
    }
}
