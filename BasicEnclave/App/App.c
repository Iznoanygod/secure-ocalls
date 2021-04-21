#include <stdlib.h>
#include <stdio.h>
#include "Enclave_u.h"
#include "sgx_urts.h"

sgx_enclave_id_t global_eid = 0;

void ocall_print(const char* str) {
	printf("%s\n", str);
}

int initialize_enclave(char* enclave_file){
	sgx_status_t = SGX_ERROR_UNEXPECTED;
	ret = sgx_create_enclave(enclave_file, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
	if (ret != SGX_SUCCESS) {
		printf("Enclave initialization error %d\n", ret);
		return -1;
	}
	return 0;
}

int SGX_CDECL main (int argc, char** argv){
	if(initialize_enclave("enclave.signed.so") < 0) {
		return -1;
	}
	printf("Performing 8349 * 13764\n");
	int res;
	sgx_status_t = secure_multiply(&res);
	printf("result: %d\n", res);
	return 0;
}
