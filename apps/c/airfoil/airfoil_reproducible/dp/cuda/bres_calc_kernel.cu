//
// auto-generated by op2.py
//

__constant__ int opDat0_bres_calc_stride_OP2CONSTANT;
int opDat0_bres_calc_stride_OP2HOST=-1;
__constant__ int opDat2_bres_calc_stride_OP2CONSTANT;
int opDat2_bres_calc_stride_OP2HOST=-1;
//user function
__device__ void bres_calc_gpu( const double *x1, const double *x2, const double *q1,
                      const double *adt1, double *res1, const int *bound) {
  double dx, dy, mu, ri, p1, vol1, p2, vol2, f;

  dx = x1[(0)*opDat0_bres_calc_stride_OP2CONSTANT] - x2[(0)*opDat0_bres_calc_stride_OP2CONSTANT];
  dy = x1[(1)*opDat0_bres_calc_stride_OP2CONSTANT] - x2[(1)*opDat0_bres_calc_stride_OP2CONSTANT];

  ri = 1.0f / q1[(0)*opDat2_bres_calc_stride_OP2CONSTANT];
  p1 = gm1_cuda * (q1[(3)*opDat2_bres_calc_stride_OP2CONSTANT] - 0.5f * ri * (q1[(1)*opDat2_bres_calc_stride_OP2CONSTANT] * q1[(1)*opDat2_bres_calc_stride_OP2CONSTANT] + q1[(2)*opDat2_bres_calc_stride_OP2CONSTANT] * q1[(2)*opDat2_bres_calc_stride_OP2CONSTANT]));

  if (*bound == 1) {
    res1[(1)*opDat2_bres_calc_stride_OP2CONSTANT] += +p1 * dy;
    res1[(2)*opDat2_bres_calc_stride_OP2CONSTANT] += -p1 * dx;
  } else {
    vol1 = ri * (q1[(1)*opDat2_bres_calc_stride_OP2CONSTANT] * dy - q1[(2)*opDat2_bres_calc_stride_OP2CONSTANT] * dx);

    ri = 1.0f / qinf_cuda[0];
    p2 = gm1_cuda * (qinf_cuda[3] - 0.5f * ri * (qinf_cuda[1] * qinf_cuda[1] + qinf_cuda[2] * qinf_cuda[2]));
    vol2 = ri * (qinf_cuda[1] * dy - qinf_cuda[2] * dx);

    mu = (*adt1) * eps_cuda;

    f = 0.5f * (vol1 * q1[(0)*opDat2_bres_calc_stride_OP2CONSTANT] + vol2 * qinf_cuda[0]) + mu * (q1[(0)*opDat2_bres_calc_stride_OP2CONSTANT] - qinf_cuda[0]);
    res1[(0)*opDat2_bres_calc_stride_OP2CONSTANT] += f;
    f = 0.5f * (vol1 * q1[(1)*opDat2_bres_calc_stride_OP2CONSTANT] + p1 * dy + vol2 * qinf_cuda[1] + p2 * dy) +
        mu * (q1[(1)*opDat2_bres_calc_stride_OP2CONSTANT] - qinf_cuda[1]);
    res1[(1)*opDat2_bres_calc_stride_OP2CONSTANT] += f;
    f = 0.5f * (vol1 * q1[(2)*opDat2_bres_calc_stride_OP2CONSTANT] - p1 * dx + vol2 * qinf_cuda[2] - p2 * dx) +
        mu * (q1[(2)*opDat2_bres_calc_stride_OP2CONSTANT] - qinf_cuda[2]);
    res1[(2)*opDat2_bres_calc_stride_OP2CONSTANT] += f;
    f = 0.5f * (vol1 * (q1[(3)*opDat2_bres_calc_stride_OP2CONSTANT] + p1) + vol2 * (qinf_cuda[3] + p2)) +
        mu * (q1[(3)*opDat2_bres_calc_stride_OP2CONSTANT] - qinf_cuda[3]);
    res1[(3)*opDat2_bres_calc_stride_OP2CONSTANT] += f;
  }

}

// CUDA kernel function
__global__ void op_cuda_bres_calc(
  const double *__restrict ind_arg0,
  const double *__restrict ind_arg1,
  const double *__restrict ind_arg2,
  double *__restrict ind_arg3,
  const int *__restrict opDat0Map,
  const int *__restrict opDat2Map,
  const int *__restrict arg5,
  int start,
  int end,
  int *col_reord,
  int   set_size) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  if (tid + start < end) {
    int n = col_reord[tid + start];
    //initialise local variables
    int map0idx;
    int map1idx;
    int map2idx;
    map0idx = opDat0Map[n + set_size * 0];
    map1idx = opDat0Map[n + set_size * 1];
    map2idx = opDat2Map[n + set_size * 0];

    //user-supplied kernel call
    bres_calc_gpu(ind_arg0+map0idx,
              ind_arg0+map1idx,
              ind_arg1+map2idx,
              ind_arg2+map2idx*1,
              ind_arg3+map2idx,
              arg5+n*1);
  }
}


//host stub function
void op_par_loop_bres_calc(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3,
  op_arg arg4,
  op_arg arg5){

  int nargs = 6;
  op_arg args[6];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  args[4] = arg4;
  args[5] = arg5;

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(3);
  op_timers_core(&cpu_t1, &wall_t1);
  OP_kernels[3].name      = name;
  OP_kernels[3].count    += 1;


  int    ninds   = 4;
  int    inds[6] = {0,0,1,2,3,-1};

  if (OP_diags>2) {
    printf(" kernel routine with indirection: bres_calc\n");
  }

  //get plan
  #ifdef OP_PART_SIZE_3
    int part_size = OP_PART_SIZE_3;
  #else
    int part_size = OP_part_size;
  #endif

  int set_size = op_mpi_halo_exchanges_cuda(set, nargs, args);
  op_map prime_map = arg4.map;
  op_reversed_map rev_map = OP_reversed_map_list[prime_map->index];

  if (set->size > 0 && rev_map != NULL ) {

    if ((OP_kernels[3].count==1) || (opDat0_bres_calc_stride_OP2HOST != getSetSizeFromOpArg(&arg0))) {
      opDat0_bres_calc_stride_OP2HOST = getSetSizeFromOpArg(&arg0);
      cudaMemcpyToSymbol(opDat0_bres_calc_stride_OP2CONSTANT, &opDat0_bres_calc_stride_OP2HOST,sizeof(int));
    }
    if ((OP_kernels[3].count==1) || (opDat2_bres_calc_stride_OP2HOST != getSetSizeFromOpArg(&arg2))) {
      opDat2_bres_calc_stride_OP2HOST = getSetSizeFromOpArg(&arg2);
      cudaMemcpyToSymbol(opDat2_bres_calc_stride_OP2CONSTANT, &opDat2_bres_calc_stride_OP2HOST,sizeof(int));
    }
    op_mpi_wait_all_cuda(nargs, args);
    //execute plan
    op_mpi_wait_all_cuda(nargs, args);
    for ( int col=0; col<rev_map->number_of_colors; col++ ){
      #ifdef OP_BLOCK_SIZE_3
      int nthread = OP_BLOCK_SIZE_3;
      #else
      int nthread = OP_block_size;
      #endif

      int start = rev_map->color_based_exec_row_starts[col];
      int end = rev_map->color_based_exec_row_starts[col+1];
      int nblocks = (end - start - 1)/nthread + 1;
      op_cuda_bres_calc<<<nblocks,nthread>>>(
      (double *)arg0.data_d,
      (double *)arg2.data_d,
      (double *)arg3.data_d,
      (double *)arg4.data_d,
      arg0.map_data_d,
      arg2.map_data_d,
      (int*)arg5.data_d,
      start,
      end,
      rev_map->color_based_exec_d,
      set->size+set->exec_size);

    }
  }
  op_mpi_set_dirtybit_cuda(nargs, args);
  cutilSafeCall(cudaDeviceSynchronize());
  //update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[3].time     += wall_t2 - wall_t1;
}
