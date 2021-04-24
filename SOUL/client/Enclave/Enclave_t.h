#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void enclave_run(void);
void enclave_add_function(const char* name, void* fcnptr, int size, const char* hash);

sgx_status_t SGX_CDECL ocall_print(const char* str);
sgx_status_t SGX_CDECL ocall_getSym(void** retval, const char* str);
sgx_status_t SGX_CDECL ocall_runfcn(int* retval, void* fcnptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
