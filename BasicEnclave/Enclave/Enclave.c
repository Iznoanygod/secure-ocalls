#include "Enclave_t.h"

int secure_multiply(int a, int b, int* res) {
	ocall_print("Enclave function called");
	*res = a * b;
	return 42;
}
