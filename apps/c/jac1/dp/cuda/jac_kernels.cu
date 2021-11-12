//
// auto-generated by op2.py
//

//global constants
#ifndef MAX_CONST_SIZE
#define MAX_CONST_SIZE 128
#endif

__constant__ double alpha_cuda;

//header
#ifndef OP_FUN_PREFIX
#define OP_FUN_PREFIX __host__ __device__
#endif
#include "../user_types.h"
#include "op_lib_cpp.h"
#include "op_cuda_rt_support.h"
#include "op_cuda_reduction.h"

void op_decl_const_char(int dim, char const *type,
int size, char *dat, char const *name){
  if (!OP_hybrid_gpu) return;
  if (!strcmp(name,"alpha")) {
    cutilSafeCall(cudaMemcpyToSymbol(alpha_cuda, dat, dim*size));
  }
  else
  {
    printf("error: unknown const name\n"); exit(1);
  }
}

//user kernel files
#include "res_kernel.cu"
#include "update_kernel.cu"