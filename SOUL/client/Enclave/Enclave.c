#include "Enclave_t.h"
#include "tlibc/string.h"
#include "tlibc/stdio.h"
#include "sgx_tcrypto.h"

/* Runs all functions of the retrived library
 * Also checks the validity of the library
 */

int loadedFunctions = 0;

typedef struct functionTable{
    void* fcnptr;
    int size;
    char* hash;
    char* name;
}functionTable;

functionTable fTable[10];

void enclave_add_function(const char* name, void* fcnptr, int size, const char* hash){
    fTable[loadedFunctions].fcnptr = (void*) fcnptr;
    fTable[loadedFunctions].size = size;
    char* hashV = malloc(40);
    memcpy(hashV, hash, 40);
    char* nameV = malloc(strlen(name));
    memcpy(nameV, name, strlen(name));
    nameV[strlen(name)] = '\0';
    fTable[loadedFunctions].hash = hashV;
    fTable[loadedFunctions].name = nameV;
    loadedFunctions++;
}
void enclave_run(){
    ocall_print("Enclave launched...\n\n");
    char buffer[512];
    for(int i = 1; i < loadedFunctions; i++){
        void* fcnptr = fTable[i].fcnptr;
        int size = fTable[i].size;
        char* func_digest = fTable[i].hash;
        char func_hash[20];
        char func_hash_dig[41];
        char* func_name = fTable[i].name;
        sgx_sha1_msg(fcnptr, size, func_hash);
        for(int j = 0; j < 20; j++){
            snprintf(func_hash_dig + 2*j, 40, "%02x", func_hash[j] & 0xff);
        }
        snprintf(buffer, 512, "Computed hash of %s: %s\n", func_name, func_hash_dig);
        ocall_print(buffer);
        ocall_print("Checking hash of function\n");
        
        if(strncmp(func_hash_dig, func_digest, 40)){
            ocall_print("Function hashes did not match, library was modified!\n\n");
            snprintf(buffer, 512, "%s was modified\n",func_name);
            continue;
        }
        ocall_print("Function hashes match, library function was not modified\n");
        ocall_print("Performing OCall of library function\n");
        int return_value;
        ocall_runfcn(&return_value, fcnptr);
        snprintf(buffer, 512, "%s: %d\n", func_name, return_value);
        ocall_print(buffer);
        ocall_print("\n");
    }
}
