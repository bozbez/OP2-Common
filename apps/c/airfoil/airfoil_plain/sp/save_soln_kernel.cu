//
// auto-generated by op2.py
//

//user function
__device__
inline void save_soln_gpu(const float *q, float *qold){
  for (int n=0; n<4; n++) qold[n] = q[n];
}


// CUDA kernel function
__global__ void op_cuda_save_soln(
  const float *__restrict arg0,
  float *arg1,
  int   set_size ) {


  //process set elements
  for ( int n=threadIdx.x+blockIdx.x*blockDim.x; n<set_size; n = n+=blockDim.x*gridDim.x ){

    //user-supplied kernel call
    save_soln_gpu(arg0+n*4,
              arg1+n*4);
  }
}


//GPU host stub function
void op_par_loop_save_soln_gpu(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1){

  int nargs = 2;
  op_arg args[2];

  args[0] = arg0;
  args[1] = arg1;

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(0);
  op_timers_core(&cpu_t1, &wall_t1);
  OP_kernels[0].name      = name;
  OP_kernels[0].count    += 1;
  if (OP_kernels[0].count==1) op_register_strides();


  if (OP_diags>2) {
    printf(" kernel routine w/o indirection:  save_soln");
  }

  op_mpi_halo_exchanges_cuda(set, nargs, args);
  if (set->size > 0) {

    //set CUDA execution parameters
    #ifdef OP_BLOCK_SIZE_0
      int nthread = OP_BLOCK_SIZE_0;
    #else
      int nthread = OP_block_size;
    //  int nthread = 128;
    #endif

    int nblocks = 200;

    op_cuda_save_soln<<<nblocks,nthread>>>(
      (float *) arg0.data_d,
      (float *) arg1.data_d,
      set->size );
  }
  op_mpi_set_dirtybit_cuda(nargs, args);
  cutilSafeCall(cudaDeviceSynchronize());
  //update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[0].time     += wall_t2 - wall_t1;
  OP_kernels[0].transfer += (float)set->size * arg0.size;
  OP_kernels[0].transfer += (float)set->size * arg1.size;
}

void op_par_loop_save_soln_cpu(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1);


//GPU host stub function
#if OP_HYBRID_GPU
void op_par_loop_save_soln(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1){

  if (OP_hybrid_gpu) {
    op_par_loop_save_soln_gpu(name, set,
      arg0,
      arg1);

    }else{
    op_par_loop_save_soln_cpu(name, set,
      arg0,
      arg1);

  }
}
#else
void op_par_loop_save_soln(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1){

  op_par_loop_save_soln_gpu(name, set,
    arg0,
    arg1);

  }
#endif //OP_HYBRID_GPU
