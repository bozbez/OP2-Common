# OP2
OP2 is a high-level embedded domain specific language for writing **unstructured mesh** algorithms with automatic parellelisation on multi-core and many-core architectures. The API is embedded in both C/C++ and Fortran.

This repository contains the implementation of the code translation tools and run-time support libraries, and is structured as follows:
 * `op2`: The C/C++ OP2 run-time libraries and Fortran bindings.
 * `translator`: The Python code translators for both C/C++ and Fortran.
 * `apps`: Example applications that demonstrate use of the API.
 * `makefiles`: Shared infrastructure of the GNU Make based build-system.
 * `doc`: LaTeX documentation source.

## Documentation
Documentation is currently available as a series of PDFs:
 * [C++ User's Manual](https://op-dsl.github.io/docs/OP2/C++_Users_Guide.pdf): An overview of the C/C++ OP2 API, intended for those looking to use OP2.
 * [Airfoil App Walkthrough](https://op-dsl.github.io/docs/OP2/airfoil-doc.pdf): A walkthrough of the `apps/{c, fortran}/airfoil` example application.
 * [OP2 Developer's Guide](https://op-dsl.github.io/docs/OP2/dev.pdf): An overview of the algorithms and implementation details of OP2, intended for those who are developing OP2.
 * [OP2 MPI Developer's Guide](https://op-dsl.github.io/docs/OP2/mpi-dev.pdf): An extension to the main developer's guide that further details issues relating to MPI.

## Quick-start
Firstly, OP2 has a varienty of dependencies that you will need depending on your use case:
 * GNU Make > 4.2
 * A C/C++ compiler: Currently supported compilers are GCC, Clang, Cray, Intel, IBM XL and NVHPC.
 * (Optional) A Fortran compiler: Currently supported compilers are GFortran, Cray, Intel, IBM XL and NVHPC.
 * (Optional) An MPI implementation: Any implementation with the `mpicc`, `mpicxx`, and `mpif90` wrappers is supported.
 * (Optional) NVIDIA CUDA > 9.2

Secondly, OP2 has a few dependencies that you must obtain and build, ideally with the same compiler you will use for OP2 and the application you are integrating OP2 into:
 * [(PT-)Scotch](https://www.labri.fr/perso/pelegrin/scotch/): Used for mesh partitioning. You must build both the sequential Scotch and parallel PT-Scotch.
 * [ParMETIS](http://glaros.dtc.umn.edu/gkhome/metis/parmetis/overview): Used for mesh partitioning.
 * (Optional) [HDF5](https://www.hdfgroup.org/solutions/hdf5/): Used for HDF5 I/O. You may build with and without `--enable-parallel` (depending on if you need MPI), and then specify both builds via the environment variables listed below.

Finally, to build OP2 and any of the apps:
 1. Set either `OP2_COMPILER={gnu, cray, intel, xl, nvhpc}`, or `OP2_{C, C_CUDA, F}_COMPILER={...}` depending on your compiler setup. Alternatively if there is a profile specific to the cluster you are building on in `makefiles/profiles` you may use e.g. `OP2_PROFILE=cirrus-intel`.
 2. Set `PTSCOTCH_INSTALL_PATH`, `PARMETIS_INSTALL_PATH`, and `HDF5_{SEQ, PAR}_INSTALL_PATH` to the locations of the respective dependency builds containing `include` and `lib` folders.
 3. (Optional) Set `CUDA_INSTALL_PATH` to the location of the installed CUDA toolkit.
 4. Run `make -j$(nproc)` in the `op2` directory to build the run-time libraries.
 5. Run `make -j$(nproc)` in any of the app directories to build the respective apps.

## Citing
To cite OP2, please reference the following paper:

[G. R. Mudalige, M. B. Giles, I. Reguly, C. Bertolli and P. H. J. Kelly, "OP2: An active library framework for solving unstructured mesh-based applications on multi-core and many-core architectures," 2012 Innovative Parallel Computing (InPar), 2012, pp. 1-12, doi: 10.1109/InPar.2012.6339594.](https://ieeexplore.ieee.org/document/6339594)

```
@INPROCEEDINGS{6339594,
  author={Mudalige, G.R. and Giles, M.B. and Reguly, I. and Bertolli, C. and Kelly, P.H.J},
  booktitle={2012 Innovative Parallel Computing (InPar)},
  title={OP2: An active library framework for solving unstructured mesh-based applications on multi-core and many-core architectures},
  year={2012},
  volume={},
  number={},
  pages={1-12},
  doi={10.1109/InPar.2012.6339594}}
```
