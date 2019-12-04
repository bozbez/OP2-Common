//
// auto-generated by op2.py
//

#include "op_lib_cpp.h"
//global_constants - values #defined by JIT
#include "jit_const.h

//user function
__device__ void res_calc_gpu( const double *x1, const double *x2, const double *q1,
                     const double *q2, const double *adt1, const double *adt2,
                     double *res1, double *res2)
{
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

//C CUDA kernel function
__global__ void op_cuda_res_calc(
 const double* __restrict ind_arg0,
 const double* __restrict ind_arg1,
 const double* __restrict ind_arg2,
 double* __restrict ind_arg3,
 const int* __restrict opDat2Map,
 const int* __restrict opDat4Map,
 int start
 int end
 int set_size)
{
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  if (tid + start < end) {
    int n = tid + start;
    //Initialise locals
    double arg6_1[4]
    for (int d = 0; d < 4; ++d)
    {
      arg6_1[d]=ZERO_double;
    }
    double arg7_1[4]
    for (int d = 0; d < 4; ++d)
    {
      arg7_1[d]=ZERO_double;
    }
    int map0idx;
    map0idx = opDat0Map[n + set_size * 0];
    int map1idx;
    map1idx = opDat0Map[n + set_size * 1];
    int map2idx;
    map2idx = opDat2Map[n + set_size * 0];
    int map3idx;
    map3idx = opDat2Map[n + set_size * 1];

    //user function call
    res_calc_gpu(ind_arg0+map0idx*2,
                 ind_arg0+map1idx*2,
                 ind_arg1+map2idx*4,
                 ind_arg1+map3idx*4,
                 ind_arg2+map2idx*1,
                 ind_arg2+map3idx*1,
                 arg6_1,
                 arg7_1
    );

    atomicAdd(&ind_arg3[0+map2idx*4],arg6_1[0]);
    atomicAdd(&ind_arg3[1+map2idx*4],arg6_1[1]);
    atomicAdd(&ind_arg3[2+map2idx*4],arg6_1[2]);
    atomicAdd(&ind_arg3[3+map2idx*4],arg6_1[3]);
    atomicAdd(&ind_arg3[0+map3idx*4],arg7_1[0]);
    atomicAdd(&ind_arg3[1+map3idx*4],arg7_1[1]);
    atomicAdd(&ind_arg3[2+map3idx*4],arg7_1[2]);
    atomicAdd(&ind_arg3[3+map3idx*4],arg7_1[3]);
  }
}

