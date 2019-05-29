//
// auto-generated by op2.py
//

// global constants
extern double gam;
extern double gm1;
extern double gm1i;
extern double m2;
extern double wtg1[2];
extern double xi1[2];
extern double Ng1[4];
extern double Ng1_xi[4];
extern double wtg2[4];
extern double Ng2[16];
extern double Ng2_xi[32];
extern double minf;
extern double freq;
extern double kappa;
extern double nmode;
extern double mfan;

// header
#include "op_lib_cpp.h"

// user kernel files
#include "res_calc_kernel.cpp"
#include "dirichlet_kernel.cpp"
#include "init_cg_kernel.cpp"
#include "spMV_kernel.cpp"
#include "dotPV_kernel.cpp"
#include "updateUR_kernel.cpp"
#include "dotR_kernel.cpp"
#include "updateP_kernel.cpp"
#include "update_kernel.cpp"
