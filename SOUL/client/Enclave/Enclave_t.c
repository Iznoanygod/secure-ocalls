#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


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

static sgx_status_t SGX_CDECL sgx_enclave_run(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	enclave_run();
	return status;
}

static sgx_status_t SGX_CDECL sgx_enclave_add_function(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_enclave_add_function_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_enclave_add_function_t* ms = SGX_CAST(ms_enclave_add_function_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	const char* _tmp_name = ms->ms_name;
	size_t _len_name = ms->ms_name_len ;
	char* _in_name = NULL;
	void* _tmp_fcnptr = ms->ms_fcnptr;
	const char* _tmp_hash = ms->ms_hash;
	size_t _len_hash = ms->ms_hash_len ;
	char* _in_hash = NULL;

	CHECK_UNIQUE_POINTER(_tmp_name, _len_name);
	CHECK_UNIQUE_POINTER(_tmp_hash, _len_hash);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_name != NULL && _len_name != 0) {
		_in_name = (char*)malloc(_len_name);
		if (_in_name == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_name, _len_name, _tmp_name, _len_name)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

		_in_name[_len_name - 1] = '\0';
		if (_len_name != strlen(_in_name) + 1)
		{
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_tmp_hash != NULL && _len_hash != 0) {
		_in_hash = (char*)malloc(_len_hash);
		if (_in_hash == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_hash, _len_hash, _tmp_hash, _len_hash)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

		_in_hash[_len_hash - 1] = '\0';
		if (_len_hash != strlen(_in_hash) + 1)
		{
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

	enclave_add_function((const char*)_in_name, _tmp_fcnptr, ms->ms_size, (const char*)_in_hash);

err:
	if (_in_name) free(_in_name);
	if (_in_hash) free(_in_hash);
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[2];
} g_ecall_table = {
	2,
	{
		{(void*)(uintptr_t)sgx_enclave_run, 0, 0},
		{(void*)(uintptr_t)sgx_enclave_add_function, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[3][2];
} g_dyn_entry_table = {
	3,
	{
		{0, 0, },
		{0, 0, },
		{0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_print(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_t));
	ocalloc_size -= sizeof(ms_ocall_print_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getSym(void** retval, const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_getSym_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getSym_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getSym_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getSym_t));
	ocalloc_size -= sizeof(ms_ocall_getSym_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	status = sgx_ocall(1, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_runfcn(int* retval, void* fcnptr)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_runfcn_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_runfcn_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_runfcn_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_runfcn_t));
	ocalloc_size -= sizeof(ms_ocall_runfcn_t);

	ms->ms_fcnptr = fcnptr;
	status = sgx_ocall(2, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

