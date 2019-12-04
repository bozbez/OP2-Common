//
// auto-generated by op2.py
//

//user function
__device__ void save_soln_gpu( const double *q, double *qold)
{
  for (int n = 0; n < 4; n++)
    qold[n] = q[n];

}

//C CUDA kernel function
__global__ void op_cuda_save_soln(
 const double* __restrict arg0,
 double* __restrict arg1,
 int set_size)
{
  //Process set elements
  for (int n = threadIdx.x+blockIdx.x*blockDim.x; n < set_size; n += blockDim.x*gridDim.x)
  {

    //user function call
    save_soln_gpu(arg0+n*4,
                  arg1+n*4
    );

  }
}

