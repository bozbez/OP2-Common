//
<<<<<<< HEAD
<<<<<<< HEAD
// auto-generated by op2.py on 2015-07-08 13:58
=======
// auto-generated by op2.py on 2015-06-22 15:49
>>>>>>> bc44746... Fixes for cuda code generation
=======
// auto-generated by op2.py
>>>>>>> 36a2362... Regenerating vectorized versions for airfoil_hdf5
//

// header
#include "op_lib_cpp.h"

// global constants
extern double gam;
extern double gm1;
extern double cfl;
extern double eps;
extern double mach;
extern double alpha;
extern double qinf[4];
// user kernel files
#include "save_soln_kernel.cpp"
#include "adt_calc_kernel.cpp"
#include "res_calc_kernel.cpp"
#include "bres_calc_kernel.cpp"
#include "update_kernel.cpp"
