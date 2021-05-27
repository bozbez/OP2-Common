//
// auto-generated by op2.py
//

//user function
//user function
//#pragma acc routine
inline void update( double *phim, double *res, const double *u, double *rms) {
  *phim -= *u;
  *res = 0.0;
  *rms += (*u) * (*u);
}

// host stub function
void op_par_loop_update(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3){

  double*arg3h = (double *)arg3.data;
  int nargs = 4;
  op_arg args[4];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(8);
  op_timers_core(&cpu_t1, &wall_t1);
  OP_kernels[8].name      = name;
  OP_kernels[8].count    += 1;


  if (OP_diags>2) {
    printf(" kernel routine w/o indirection:  update");
  }

  op_mpi_halo_exchanges_cuda(set, nargs, args);

  double arg3_l = arg3h[0];

  if (set->size >0) {


    //Set up typed device pointers for OpenACC

    double* data0 = (double*)arg0.data_d;
    double* data1 = (double*)arg1.data_d;
    double* data2 = (double*)arg2.data_d;
    #pragma acc parallel loop independent deviceptr(data0,data1,data2) reduction(+:arg3_l)
    for ( int n=0; n<set->size; n++ ){
      update(
        &data0[1*n],
        &data1[1*n],
        &data2[1*n],
        &arg3_l);
    }
  }

  // combine reduction data
  arg3h[0] = arg3_l;
  op_mpi_reduce_double(&arg3,arg3h);
  op_mpi_set_dirtybit_cuda(nargs, args);

  // update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[8].time     += wall_t2 - wall_t1;
  OP_kernels[8].transfer += (float)set->size * arg0.size * 2.0f;
  OP_kernels[8].transfer += (float)set->size * arg1.size * 2.0f;
  OP_kernels[8].transfer += (float)set->size * arg2.size;
}