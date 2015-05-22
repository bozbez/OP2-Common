//
// auto-generated by op2.py on 2015-07-08 13:58
//

//user function
//#include "update.h"
#include "op_vector.h"
inline void update(const double *qold, double *q, double *res, const double *adt, double *rms){
  double del, adti;

  adti = 1.0f/(*adt);

  for (int n=0; n<4; n++) {
    del    = adti*res[n];
    q[n]   = qold[n] - del;
    res[n] = 0.0f;
    *rms  += del*del;
  }
}

#ifdef VECTORIZE
#define SIMD_VEC 4
inline void update_vec(const double qold[*][SIMD_VEC], double q[*][SIMD_VEC],
  double res[*][SIMD_VEC], const double adt[*][SIMD_VEC], double rms[SIMD_VEC],
  int idx){

  double del, adti;

  adti = 1.0f/(adt[0][idx]);

  for (int n=0; n<4; n++) {
    del    = adti*res[n][idx];
    q[n][idx]   = qold[n][idx] - del;
    res[n][idx] = 0.0f;
    rms[idx]  += del*del;
  }
}
#endif

// host stub function
void op_par_loop_update(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3,
  op_arg arg4){

  int nargs = 5;
  op_arg args[5];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  args[4] = arg4;

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(4);
  op_timers_core(&cpu_t1, &wall_t1);


  if (OP_diags>2) {
    printf(" kernel routine w/o indirection:  update");
  }

  int exec_size = op_mpi_halo_exchanges(set, nargs, args);
  int set_size = ((set->size+set->exec_size-1)/16+1)*16; //align to 512 bits

  if (exec_size >0) {

#ifdef VECTORIZE
    #pragma novector
    for ( int n=0; n<0+(exec_size/SIMD_VEC)*SIMD_VEC; n+=SIMD_VEC ){
      double dat4[SIMD_VEC];

      /*double dat0[4][SIMD_VEC];
      double dat1[4][SIMD_VEC];
      double dat2[4][SIMD_VEC];
      double dat3[1][SIMD_VEC];
      double dat4[SIMD_VEC];

      #pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){

        dat0[0][i] = ((double*)arg0.data)[(n+i) * 4 + 0];
        dat0[1][i] = ((double*)arg0.data)[(n+i) * 4 + 1];
        dat0[2][i] = ((double*)arg0.data)[(n+i) * 4 + 2];
        dat0[3][i] = ((double*)arg0.data)[(n+i) * 4 + 3];

        dat2[0][i] = ((double*)arg2.data)[(n+i) * 4 + 0];
        dat2[1][i] = ((double*)arg2.data)[(n+i) * 4 + 1];
        dat2[2][i] = ((double*)arg2.data)[(n+i) * 4 + 2];
        dat2[3][i] = ((double*)arg2.data)[(n+i) * 4 + 3];

        dat3[0][i] = ((int*)arg3.data)[(n+i) * 1 + 0];

        //dat4[i] = 0.0;

      }*/
      #pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){
        //update_vec(dat0, dat1, dat2, dat3, dat4, i);
        update(
        &((double*)arg0.data)[(n+i) * 4],
        &((double*)arg1.data)[(n+i) * 4],
        &((double*)arg2.data)[(n+i) * 4],
        &((double*)arg3.data)[(n+i) * 1],
        //(double*)arg4.data);
        &dat4[i]);
      }
      *(double*)arg4.data += add_horizontal(dat4);
      /*for ( int i=0; i<SIMD_VEC; i++ ){
        *(double*)arg4.data += dat4[i];
      }*/

      /*#pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){
        ((double*)arg1.data)[(n+i) * 4 + 0] = dat1[0][i];
        ((double*)arg1.data)[(n+i) * 4 + 1] = dat1[1][i];
        ((double*)arg1.data)[(n+i) * 4 + 2] = dat1[2][i];
        ((double*)arg1.data)[(n+i) * 4 + 3] = dat1[3][i];

        ((double*)arg2.data)[(n+i) * 4 + 0] = dat2[0][i];
        ((double*)arg2.data)[(n+i) * 4 + 1] = dat2[1][i];
        ((double*)arg2.data)[(n+i) * 4 + 2] = dat2[2][i];
        ((double*)arg2.data)[(n+i) * 4 + 3] = dat2[3][i];

        //*(double*)arg4.data += dat4[i];
      }*/
    }

//remainder
    for ( int n=(exec_size/SIMD_VEC)*SIMD_VEC; n<exec_size; n++ ){
#else
    for ( int n=0; n<exec_size; n++ ){
#endif
      update(
        &((double*)arg0.data)[4*n],
        &((double*)arg1.data)[4*n],
        &((double*)arg2.data)[4*n],
        &((double*)arg3.data)[1*n],
        (double*)arg4.data);
    }
  }

  // combine reduction data
  op_mpi_reduce(&arg4,(double*)arg4.data);
  op_mpi_set_dirtybit(nargs, args);

  // update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[4].name      = name;
  OP_kernels[4].count    += 1;
  OP_kernels[4].time     += wall_t2 - wall_t1;
  OP_kernels[4].transfer += (float)set->size * arg0.size;
  OP_kernels[4].transfer += (float)set->size * arg1.size * 2.0f;
  OP_kernels[4].transfer += (float)set->size * arg2.size * 2.0f;
  OP_kernels[4].transfer += (float)set->size * arg3.size;
}
