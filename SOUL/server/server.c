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
    sprintf(command, "gcc library.c -o library.so -O0 -fPIC -shared -D RANDOM_NUM=%d", rval);
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
        srand(time(0));
        int randomvalue = rand();
        while(1){
            printf("Waiting for command\n");
            char* message = getMessage(sock);
            if(!strcmp(message, "disconnect")){
                free(message);
                remove("./library.so");
                //goto loopend;
                break;
            }
            else if(!strcmp(message, "getLibrary")){
                int status = compileLibrary(randomvalue);
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
            else if(!strcmp(message, "getHash")){
                if(access("library.so", F_OK)) {
                    printf("No dump file\n");
                    write(sock, "0:", 2);
                    break;
                }
                char command[512];
                system("objdump -T library.so | grep -e string_print_test -e response -e basic_return_test > table.odmp");
                int dmpfd = open("table.odmp", O_RDONLY);
                int size = lseek(dmpfd, 0, SEEK_END);
                lseek(dmpfd, 0, SEEK_SET);
                char* dmp = malloc(size + 1);
                simpleRead(dmpfd, dmp, size);
                dmp[size] = '\0';
                close(dmpfd);
                void* handle = dlopen("./library.so", RTLD_NOW);
                int fsize;
                char name[512];
                //system(command);
                FILE* hdmp = fopen("hash.dmp", "w");
                int status;
                int bytesread = 0;
                while(sscanf(dmp + bytesread, "%*s %*c %*s %*s %x %*s %s\n%n", &fsize, name, &status) != EOF){
                    bytesread += status;
                    void* ptr = dlsym(handle, name);
                    unsigned char hash[20];
                    SHA1(ptr, fsize, hash);
                    //sprintf(command, "echo %s %s >> hash.dmp", name, hash);
                    //system(command);
                    fprintf(hdmp, "%s %d ", name, fsize);
                    for (int i = 0; i < 20; i++) {
                        fprintf(hdmp, "%02x", hash[i]);
                    }
                    fprintf(hdmp, "\n");
                }
                fclose(hdmp);
                int ffd = open("hash.dmp", O_RDONLY);
                int ffs = lseek(ffd, 0, SEEK_END);
                lseek(ffd, 0, SEEK_SET);
                char* fbuff = malloc(ffs);
                simpleRead(ffd, fbuff, ffs);
                sprintf(command, "%d:", ffs);
                simpleWrite(sock, command, strlen(command));
                simpleWrite(sock, fbuff, ffs);
                free(fbuff);
                close(ffd);
                free(dmp);
                dlclose(handle);
                remove("hash.dmp");
                remove("table.odmp");
            }
            else if(!strcmp(message, "getResponse")){
                srand(randomvalue);
                int challenge = rand();
                srand(randomvalue + challenge);
                int response = rand();
                simpleWrite(sock, (char*)&challenge, sizeof(int));
                int client_r;
                simpleRead(sock, (char*)&client_r, sizeof(int));
                if(client_r != response){
                    simpleWrite(sock, "failure", 7);
                    free(message);
                    remove("./library.so");
                    //goto loopend;
                    break;
                }
                simpleWrite(sock, "success", 7);
            }
            free(message);
        }
        close(sock);
        
    }
}
