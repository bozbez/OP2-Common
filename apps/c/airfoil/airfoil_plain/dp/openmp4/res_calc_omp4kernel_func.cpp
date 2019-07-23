//
// auto-generated by op2.py
//

void res_calc_omp4_kernel(
  int *map0,
  int map0size,
  int *map2,
  int map2size,
  double *data0,
  int dat0size,
  double *data2,
  int dat2size,
  double *data4,
  int dat4size,
  double *data6,
  int dat6size,
  int *col_reord,
  int set_size1,
  int start,
  int end,
  int num_teams,
  int nthread){

  #pragma omp target teams distribute parallel for schedule(static,1)\
     num_teams(num_teams) thread_limit(nthread)  \
    map(to: gm1_ompkernel, eps_ompkernel)\
    map(to:col_reord[0:set_size1],map0[0:map0size],map2[0:map2size],data0[0:dat0size],data2[0:dat2size],data4[0:dat4size],data6[0:dat6size])
  for ( int e=start; e<end; e++ ){
    int n_op = col_reord[e];
    int map0idx = map0[n_op + set_size1 * 0];
    int map1idx = map0[n_op + set_size1 * 1];
    int map2idx = map2[n_op + set_size1 * 0];
    int map3idx = map2[n_op + set_size1 * 1];

    //variable mapping
    const double *x1 = &data0[2 * map0idx];
    const double *x2 = &data0[2 * map1idx];
    const double *q1 = &data2[4 * map2idx];
    const double *q2 = &data2[4 * map3idx];
    const double *adt1 = &data4[1 * map2idx];
    const double *adt2 = &data4[1 * map3idx];
    double *res1 = &data6[4 * map2idx];
    double *res2 = &data6[4 * map3idx];

    //inline function
    
    double dx, dy, mu, ri, p1, vol1, p2, vol2, f;

    dx = x1[0] - x2[0];
    dy = x1[1] - x2[1];

    ri = 1.0f / q1[0];
    p1 = gm1_ompkernel * (q1[3] - 0.5f * ri * (q1[1] * q1[1] + q1[2] * q1[2]));
    vol1 = ri * (q1[1] * dy - q1[2] * dx);

    ri = 1.0f / q2[0];
    p2 = gm1_ompkernel * (q2[3] - 0.5f * ri * (q2[1] * q2[1] + q2[2] * q2[2]));
    vol2 = ri * (q2[1] * dy - q2[2] * dx);

    mu = 0.5f * ((*adt1) + (*adt2)) * eps_ompkernel;

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
    //end inline func
  }

}
