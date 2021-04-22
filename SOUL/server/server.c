#include "server.h"

int compileLibrary(int rval){
    /*int pid = fork();   
    if(pid) {
        int status;
        wait(&status);
        return status;
    }
    char command[256];
    sprintf(command, "RANDOM_NUM=%d", rval);
    execl("gcc","gcc","library.c", "-o", "library.so", "-fPIC", "-shared", "-D", command, NULL);
*/
    char command[256];
    sprintf(command, "gcc library.c -o library.so -fPIC -shared -D RANDOM_NUM=%d", rval);
    int status = system(command);
    return status;
}
    
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

char* getMessage(int sock){
    char buffer[512];
    int i = 0;
    while(1){
        read(sock, buffer+i, 1);
        if(buffer[i] == ':')
            break;
        i++;
    }
    buffer[i] = '\0';
    int length = atoi(buffer);
    char* message = malloc(length+1);
    simpleRead(sock, message, length);
    message[length] = '\0';
    return message;
}

int main(int argc, char** argv) {
    if(argc < 2){
        printf("usage: server port\n");
        return 1;
    }
    int port = atoi(argv[1]);
    srand(time(0));
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
        
        int randomvalue = rand();
        while(1){
            char* message = getMessage(sock);
            if(!strcmp(message, "disconnect")){
                free(message);
                remove("./library.so");
                goto loopend;
            }
            else if(!strcmp(message, "getLibrary")){
                int status = compileLibrary(randomvalue);
                printf("%d\n", status);
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
            }
            else if(!strcmp(message, "getTable")){

            }
        }
loopend:
        close(sock);
        
    }
}
