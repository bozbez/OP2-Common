//
// auto-generated by op2.py
//

// global constants
extern double gam;
extern double gm1;
extern double cfl;
extern double eps;
extern double mach;
extern double alpha;
extern double qinf[4];

<<<<<<< HEAD
// header
#include "op_lib_cpp.h"

=======
>>>>>>> Working bres_calc, adt_calc and update
// user kernel files
#include "save_soln_seqkernel.cpp"
#include "adt_calc_seqkernel.cpp"
#include "res_calc_seqkernel.cpp"
#include "bres_calc_seqkernel.cpp"
#include "update_seqkernel.cpp"

#ifdef OPS_JIT
void jit_consts() {
  FILE *f = fopen("jit_const.h", "r");
    if (f == NULL) {
      f = fopen("jit_const.h", "w"); // create only if file does not exist
      if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
      }
      /*need to generate this block of code using the code generator
      using what is declared in op_decal_consts
      */
      fprintf(f, "#define gam %lf\n", gam);
      fprintf(f, "#define gm1 %lf\n", gm1);
      fprintf(f, "#define cfl %lf\n", cfl);
      fprintf(f, "#define eps %lf\n", eps);
      fprintf(f, "#define mach %lf\n", mach);
      fprintf(f, "#define alpha %lf\n", alpha);

      fprintf(f, "extern double qinf[4];\n");

      fclose(f);
    }
}
#endif