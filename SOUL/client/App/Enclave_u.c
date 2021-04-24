#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_enclave_add_function_t {
	const char* ms_name;
	size_t ms_name_len;
	void* ms_fcnptr;
	int ms_size;
	const char* ms_hash;
	size_t ms_hash_len;
} ms_enclave_add_function_t;

typedef struct ms_ocall_print_t {
	const char* ms_str;
} ms_ocall_print_t;

typedef struct ms_ocall_getSym_t {
	void* ms_retval;
	const char* ms_str;
} ms_ocall_getSym_t;

typedef struct ms_ocall_runfcn_t {
	int ms_retval;
	void* ms_fcnptr;
} ms_ocall_runfcn_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print(void* pms)
{
	ms_ocall_print_t* ms = SGX_CAST(ms_ocall_print_t*, pms);
	ocall_print(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_getSym(void* pms)
{
	ms_ocall_getSym_t* ms = SGX_CAST(ms_ocall_getSym_t*, pms);
	ms->ms_retval = ocall_getSym(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_runfcn(void* pms)
{
	ms_ocall_runfcn_t* ms = SGX_CAST(ms_ocall_runfcn_t*, pms);
	ms->ms_retval = ocall_runfcn(ms->ms_fcnptr);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[3];
} ocall_table_Enclave = {
	3,
	{
		(void*)Enclave_ocall_print,
		(void*)Enclave_ocall_getSym,
		(void*)Enclave_ocall_runfcn,
	}
};
sgx_status_t enclave_run(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, NULL);
	return status;
}

sgx_status_t enclave_add_function(sgx_enclave_id_t eid, const char* name, void* fcnptr, int size, const char* hash)
{
	sgx_status_t status;
	ms_enclave_add_function_t ms;
	ms.ms_name = name;
	ms.ms_name_len = name ? strlen(name) + 1 : 0;
	ms.ms_fcnptr = fcnptr;
	ms.ms_size = size;
	ms.ms_hash = hash;
	ms.ms_hash_len = hash ? strlen(hash) + 1 : 0;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	return status;
}

