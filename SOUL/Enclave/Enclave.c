#include "Enclave_t.h"

int secure_multiply(int a, int b) {
	ocall_print("Enclave function called");
	return a * b;
} 
