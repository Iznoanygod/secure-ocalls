#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_DEFINED__
#define OCALL_PRINT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print, (const char* str));
#endif
#ifndef OCALL_GETSYM_DEFINED__
#define OCALL_GETSYM_DEFINED__
void* SGX_UBRIDGE(SGX_NOCONVENTION, ocall_getSym, (const char* str));
#endif
#ifndef OCALL_RUNFCN_DEFINED__
#define OCALL_RUNFCN_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_runfcn, (void* fcnptr));
#endif

sgx_status_t enclave_run(sgx_enclave_id_t eid);
sgx_status_t enclave_add_function(sgx_enclave_id_t eid, const char* name, void* fcnptr, int size, const char* hash);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
