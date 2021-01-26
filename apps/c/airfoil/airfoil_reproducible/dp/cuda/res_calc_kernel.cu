//
// auto-generated by op2.py
//

//user function
__device__ void res_calc_gpu( const double *x1, const double *x2, const double *q1,
                     const double *q2, const double *adt1, const double *adt2,
                     double *res1, double *res2) {
  double dx, dy, mu, ri, p1, vol1, p2, vol2, f;

  dx = x1[0] - x2[0];
  dy = x1[1] - x2[1];

  ri = 1.0f / q1[0];
  p1 = gm1 * (q1[3] - 0.5f * ri * (q1[1] * q1[1] + q1[2] * q1[2]));
  vol1 = ri * (q1[1] * dy - q1[2] * dx);

  ri = 1.0f / q2[0];
  p2 = gm1 * (q2[3] - 0.5f * ri * (q2[1] * q2[1] + q2[2] * q2[2]));
  vol2 = ri * (q2[1] * dy - q2[2] * dx);

  mu = 0.5f * ((*adt1) + (*adt2)) * eps;

  f = 0.5f * (vol1 * q1[0] + vol2 * q2[0]) + mu * (q1[0] - q2[0]);
  res1[0] += f;
  res2[0] -= f;
  f = 0.5f * (vol1 * q1[1] + p1 * dy + vol2 * q2[1] + p2 * dy) +
      mu * (q1[1] - q2[1]);
  res1[1] += f;
  res2[1] -= f;
  f = 0.5f * (vol1 * q1[2] - p1 * dx + vol2 * q2[2] - p2 * dx) +
      mu * (q1[2] - q2[2]);
  res1[2] += f;
  res2[2] -= f;
  f = 0.5f * (vol1 * (q1[3] + p1) + vol2 * (q2[3] + p2)) + mu * (q1[3] - q2[3]);
  res1[3] += f;
  res2[3] -= f;
}

// CUDA kernel function
__global__ void op_cuda_res_calc(
  const double *__restrict ind_arg0,
  const double *__restrict ind_arg1,
  const double *__restrict ind_arg2,
  double *__restrict ind_arg3,
  const int *__restrict opDat0Map,
  const int *__restrict opDat2Map,
  int start,
  int end,
  int *col_reord,
  int   set_size) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  if(tid + start >= end) return;
  int n = col_reord[tid + start];
  //initialise local variables
  int map0idx;
  int map1idx;
  int map2idx;
  int map3idx;
  map0idx = opDat0Map[n + set_size * 0];
  map1idx = opDat0Map[n + set_size * 1];
  map2idx = opDat2Map[n + set_size * 0];
  map3idx = opDat2Map[n + set_size * 1];

  //user-supplied kernel call
  res_calc_gpu(ind_arg0+map0idx*2,
             ind_arg0+map1idx*2,
             ind_arg1+map2idx*4,
             ind_arg1+map3idx*4,
             ind_arg2+map2idx*1,
             ind_arg2+map3idx*1,
             ind_arg3+map2idx*4,
             ind_arg3+map3idx*4);
}


//host stub function
void op_par_loop_res_calc(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3,
  op_arg arg4,
  op_arg arg5,
  op_arg arg6,
  op_arg arg7){

  int nargs = 8;
  op_arg args[8];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  args[4] = arg4;
  args[5] = arg5;
  args[6] = arg6;
  args[7] = arg7;

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(2);
  op_timers_core(&cpu_t1, &wall_t1);
  OP_kernels[2].name      = name;
  OP_kernels[2].count    += 1;


  int    ninds   = 4;
  int    inds[8] = {0,0,1,1,2,2,3,3};

  if (OP_diags>2) {
    printf(" kernel routine with indirection: res_calc\n");
  }

  //get plan
  #ifdef OP_PART_SIZE_2
    int part_size = OP_PART_SIZE_2;
  #else
    int part_size = OP_part_size;
  #endif

  int set_size = op_mpi_halo_exchanges_cuda(set, nargs, args);
  op_map prime_map = arg6.map;
  op_reversed_map rev_map = OP_reversed_map_list[prime_map->index];

  if (set->size > 0 && rev_map != NULL ) {

    op_plan *Plan = op_plan_get_stage(name,set,part_size,nargs,args,ninds,inds,OP_COLOR2);

    op_mpi_wait_all_cuda(nargs, args);
    //execute plan
    for ( int col=0; col<rev_map->number_of_colors; col++ ){
      if (col==Plan->ncolors_core) {
        op_mpi_wait_all_cuda(nargs, args);
      }
      #ifdef OP_BLOCK_SIZE_2
      int nthread = OP_BLOCK_SIZE_2;
      #else
      int nthread = OP_block_size;
      #endif

      int start = rev_map->color_based_exec_row_starts[col];
      int end = rev_map->color_based_exec_row_starts[col+1];
      int nblocks = (end - start - 1)/nthread + 1;
      op_cuda_res_calc<<<nblocks,nthread>>>(
      (double *)arg0.data_d,
      (double *)arg2.data_d,
      (double *)arg4.data_d,
      (double *)arg6.data_d,
      arg0.map_data_d,
      arg2.map_data_d,
      start,
      end,
      rev_map->color_based_exec_d,
      set->size+set->exec_size);

    }
    OP_kernels[2].transfer  += Plan->transfer;
    OP_kernels[2].transfer2 += Plan->transfer2;
  }
  op_mpi_set_dirtybit_cuda(nargs, args);
  cutilSafeCall(cudaDeviceSynchronize());
  //update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[2].time     += wall_t2 - wall_t1;
}