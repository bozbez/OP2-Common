#!/bin/bash

#reproducible bulid and test script for MPI genseq

NP=64
N=5 #number of tests

MG_CFD_INSTALL_PATH=/home/sikba/MG-CFD-app-OP2
#MG_CFD_APP_PATH=/home/sikba/Rotor37_1M
MG_CFD_APP_PATH=/home/sikba/Rotor37_8M

outputdir=${OP2_INSTALL_PATH}/../scripts/reprotests/MPI/test-NP${NP}_$(date "+%Y-%m-%d--%H_%M")
mkdir -p ${outputdir}

norepfile=${outputdir}/norepro.log
temparrayfile=${outputdir}/temparray.log
greedycolfile=${outputdir}/greedycol.log   
distfile=${outputdir}/distcol.log    
trivfile=${outputdir}/trivcol.log    
buildfile=${outputdir}/build.log    


echo "build log file: $buildfile"
echo "non reproducible file: $norepfile"
echo "temparray file: $temparrayfile"
echo "greedy coloring file: $greedycolfile "
echo "distributed coloring file: $distfile"
echo "trivial coloring file: $trivfile"

tmp_file_CPU=${OP2_INSTALL_PATH}/../scripts/reprotests/tmp_count_file_CPU
touch $tmp_file_CPU

#parameter: tmp_file_CPU
calc_runtime () {
    local runtime=$(grep -oP "Max total runtime =\w*\K.*" $1)
    local plantimes=$(grep -oP "Total plan time:\w*\K.*" $1)

    #echo $runtime
    #echo $plantimes
    avg_plantime=0
    for i in $plantimes
    do
        avg_plantime=$(awk "BEGIN {print $avg_plantime+$i; exit}")
    done
    len_plant=$(wc -w <<< "$plantimes")
    avg_plantime=$(awk "BEGIN {print $avg_plantime/$len_plant; exit}")
    runtime=$(awk "BEGIN {print $runtime-$avg_plantime; exit}")
    echo $runtime
}

#generate nonreproducible files and compile them
echo "generateing nonreproducible apps"
cd ${OP2_INSTALL_PATH}/../translator/c/python
sed -i 's/reproducible = ./reproducible = 0/g' op2_gen_common.py
sed -i 's/repr_temp_array = ./repr_temp_array = 0/g' op2_gen_common.py
sed -i 's/repr_coloring = ./repr_coloring = 0/g' op2_gen_common.py
sed -i 's/trivial_coloring = ./trivial_coloring = 0/g' op2_gen_common.py

cd ${OP2_INSTALL_PATH}/../apps/c/airfoil/airfoil_reproducible/dp
${OP2_INSTALL_PATH}/../translator/c/python/op2.py airfoil.cpp &>> $buildfile
make airfoil_mpi_genseq &>> $buildfile
mv airfoil_mpi_genseq airfoil_mpi_genseq_norepro

cd ${OP2_INSTALL_PATH}/../apps/c/aero/aero_reproducible/dp/
${OP2_INSTALL_PATH}/../translator/c/python/op2.py aero.cpp &>> $buildfile
make aero_mpi_genseq &>> $buildfile
mv aero_mpi_genseq aero_mpi_genseq_norepro

cd ${MG_CFD_INSTALL_PATH}
./translate2op2.sh &>> $buildfile
make mpi &>> $buildfile
mv bin/mgcfd_mpi bin/mgcfd_mpi_norepro


#generate temparray files and compile them
echo "generateing temparray apps"
cd ${OP2_INSTALL_PATH}/../translator/c/python
sed -i 's/reproducible = ./reproducible = 1/g' op2_gen_common.py
sed -i 's/repr_temp_array = ./repr_temp_array = 1/g' op2_gen_common.py
sed -i 's/repr_coloring = ./repr_coloring = 0/g' op2_gen_common.py
sed -i 's/trivial_coloring = ./trivial_coloring = 0/g' op2_gen_common.py

cd ${OP2_INSTALL_PATH}/../apps/c/airfoil/airfoil_reproducible/dp
${OP2_INSTALL_PATH}/../translator/c/python/op2.py airfoil.cpp &>> $buildfile
make airfoil_mpi_genseq &>> $buildfile
mv airfoil_mpi_genseq airfoil_mpi_genseq_temparray

cd ${OP2_INSTALL_PATH}/../apps/c/aero/aero_reproducible/dp/
${OP2_INSTALL_PATH}/../translator/c/python/op2.py aero.cpp &>> $buildfile
make aero_mpi_genseq &>> $buildfile
mv aero_mpi_genseq aero_mpi_genseq_temparray

cd ${MG_CFD_INSTALL_PATH}
./translate2op2.sh &>> $buildfile
make mpi &>> $buildfile
mv bin/mgcfd_mpi bin/mgcfd_mpi_temparray


#generate repr coloring files and compile them
echo "generateing repr coloring apps"
cd ${OP2_INSTALL_PATH}/../translator/c/python
sed -i 's/reproducible = ./reproducible = 1/g' op2_gen_common.py
sed -i 's/repr_temp_array = ./repr_temp_array = 0/g' op2_gen_common.py
sed -i 's/repr_coloring = ./repr_coloring = 1/g' op2_gen_common.py
sed -i 's/trivial_coloring = ./trivial_coloring = 0/g' op2_gen_common.py

cd ${OP2_INSTALL_PATH}/../apps/c/airfoil/airfoil_reproducible/dp
${OP2_INSTALL_PATH}/../translator/c/python/op2.py airfoil.cpp &>> $buildfile
make airfoil_mpi_genseq &>> $buildfile
mv airfoil_mpi_genseq airfoil_mpi_genseq_reprcolor

cd ${OP2_INSTALL_PATH}/../apps/c/aero/aero_reproducible/dp/
${OP2_INSTALL_PATH}/../translator/c/python/op2.py aero.cpp &>> $buildfile
make aero_mpi_genseq &>> $buildfile
mv aero_mpi_genseq aero_mpi_genseq_reprcolor

cd ${MG_CFD_INSTALL_PATH}
./translate2op2.sh &>> $buildfile
make mpi &>> $buildfile
mv bin/mgcfd_mpi bin/mgcfd_mpi_genseq_reprcolor

#generate trivial coloring files and compile them
echo "generateing trivial coloring apps"
cd ${OP2_INSTALL_PATH}/../translator/c/python
sed -i 's/reproducible = ./reproducible = 1/g' op2_gen_common.py
sed -i 's/repr_temp_array = ./repr_temp_array = 0/g' op2_gen_common.py
sed -i 's/repr_coloring = ./repr_coloring = 1/g' op2_gen_common.py
sed -i 's/trivial_coloring = ./trivial_coloring = 1/g' op2_gen_common.py

cd ${OP2_INSTALL_PATH}/../apps/c/airfoil/airfoil_reproducible/dp
${OP2_INSTALL_PATH}/../translator/c/python/op2.py airfoil.cpp &>> $buildfile
make airfoil_mpi_genseq &>> $buildfile
mv airfoil_mpi_genseq airfoil_mpi_genseq_trivcolor

cd ${OP2_INSTALL_PATH}/../apps/c/aero/aero_reproducible/dp/
${OP2_INSTALL_PATH}/../translator/c/python/op2.py aero.cpp &>> $buildfile
make aero_mpi_genseq &>> $buildfile
mv aero_mpi_genseq aero_mpi_genseq_trivcolor

cd ${MG_CFD_INSTALL_PATH}
./translate2op2.sh &>> $buildfile
make mpi &>> $buildfile
mv bin/mgcfd_mpi bin/mgcfd_mpi_genseq_trivcolor

echo "done"


#start of measurements


#echo "--- Creating colors for greedy coloring test ---"
#
#cd ${OP2_INSTALL_PATH}/../apps/c/airfoil/airfoil_reproducible/dp/
#rm -f *coloring.h5
#cd ${OP2_INSTALL_PATH}/../apps/c/aero/aero_reproducible/dp/
#rm -f *coloring.h5
#cd ${MG_CFD_APP_PATH}
#rm -f *coloring.h5
#
#cd ${OP2_INSTALL_PATH}/../apps/c/airfoil/airfoil_reproducible/dp/
#echo "airfoil"
#mpirun -np 1 ./airfoil_mpi_genseq_reprcolor -op_repro_greedy_coloring > /dev/null
#cd ${OP2_INSTALL_PATH}/../apps/c/aero/aero_reproducible/dp/
#echo "aero"
#mpirun -np 1 ./aero_mpi_genseq_reprcolor -op_repro_greedy_coloring > /dev/null
#cd ${MG_CFD_APP_PATH}
#echo "mg-cfd"
#mpirun -np 1 ../MG-CFD-app-OP2/bin/mgcfd_mpi_genseq_reprcolor -i input-mgcfd.dat -m ptscotch -op_repro_greedy_coloring > /dev/null



declare -a arr_airfoil_nonrepro
declare -a arr_airfoil_temparray
declare -a arr_airfoil_greedycol
declare -a arr_airfoil_distrcol
declare -a arr_airfoil_trivcol

declare -a arr_aero_nonrepro
declare -a arr_aero_temparray
declare -a arr_aero_greedycol
declare -a arr_aero_distrcol
declare -a arr_aero_trivcol

declare -a arr_mgcfd_nonrepro
declare -a arr_mgcfd_temparray
declare -a arr_mgcfd_greedycol
declare -a arr_mgcfd_distrcol
declare -a arr_mgcfd_trivcol


for i in $(eval echo "{1..$N}")
do
    echo "--- Repro measurements $i/$N ---"
    cd ${OP2_INSTALL_PATH}/../apps/c/airfoil/airfoil_reproducible/dp/
    echo -n "Airfoil_mpi_genseq np${NP}, input: "
    ls -la | grep "new_grid.h5 " | grep -o "/new_grid.*" | tr '\n' '\0'
    echo ""
    echo "norepro"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./airfoil_mpi_genseq_norepro 2>&1 | tee -a $norepfile  > $tmp_file_CPU
    arr_airfoil_nonrepro+=($(calc_runtime $tmp_file_CPU))
    echo "temparray"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./airfoil_mpi_genseq_temparray 2>&1 | tee -a $temparrayfile > $tmp_file_CPU
    arr_airfoil_temparray+=($(calc_runtime $tmp_file_CPU))
    echo "greedycol"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./airfoil_mpi_genseq_reprcolor -op_repro_greedy_coloring 2>&1 | tee -a $greedycolfile  > $tmp_file_CPU
    arr_airfoil_greedycol+=($(calc_runtime $tmp_file_CPU))
    echo "distcol"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./airfoil_mpi_genseq_reprcolor 2>&1 | tee -a $distfile  > $tmp_file_CPU
    arr_airfoil_distrcol+=($(calc_runtime $tmp_file_CPU))
    echo "trivcol"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./airfoil_mpi_genseq_trivcolor 2>&1 | tee -a $trivfile  > $tmp_file_CPU
    arr_airfoil_trivcol+=($(calc_runtime $tmp_file_CPU))
    
    cd ${OP2_INSTALL_PATH}/../apps/c/aero/aero_reproducible/dp/

    echo -n "Aero_mpi_genseq np${NP}, input: "
    ls -la | grep "FE_grid.h5 " | grep -o "/FE_grid.*" | tr '\n' '\0'
    echo ""
    echo "norepro"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./aero_mpi_genseq_norepro 2>&1 | tee -a $norepfile  > $tmp_file_CPU
    arr_aero_nonrepro+=($(calc_runtime $tmp_file_CPU))
    echo "temparray"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./aero_mpi_genseq_temparray 2>&1 | tee -a $temparrayfile > $tmp_file_CPU
    arr_aero_temparray+=($(calc_runtime $tmp_file_CPU))
    echo "greedycol"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./aero_mpi_genseq_reprcolor -op_repro_greedy_coloring 2>&1 | tee -a $greedycolfile  > $tmp_file_CPU
    arr_aero_greedycol+=($(calc_runtime $tmp_file_CPU))
    echo "distcol"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./aero_mpi_genseq_reprcolor 2>&1 | tee -a $distfile  > $tmp_file_CPU
    arr_aero_distrcol+=($(calc_runtime $tmp_file_CPU))
    echo "trivcol"
    mpirun -mca io ^ompio -np $NP ~/numawrap ./aero_mpi_genseq_trivcolor 2>&1 | tee -a $trivfile  > $tmp_file_CPU
    arr_aero_trivcol+=($(calc_runtime $tmp_file_CPU))


    cd ${MG_CFD_APP_PATH}
    echo -n "mg-cfd_mpi np${NP}, input loc: ${MG_CFD_APP_PATH}"
    echo " "
    echo "norepro"
    mpirun -mca io ^ompio -np $NP ~/numawrap  ../MG-CFD-app-OP2/bin/mgcfd_mpi_norepro -i input-mgcfd.dat -m ptscotch OP_PART_SIZE=2048 2>&1 | tee -a $norepfile > $tmp_file_CPU
    arr_mgcfd_nonrepro+=($(calc_runtime $tmp_file_CPU))
    echo "temparray"
    mpirun -mca io ^ompio -np $NP ~/numawrap  ../MG-CFD-app-OP2/bin/mgcfd_mpi_temparray -i input-mgcfd.dat -m ptscotch OP_PART_SIZE=2048 2>&1 | tee -a $temparrayfile  > $tmp_file_CPU
    arr_mgcfd_temparray+=($(calc_runtime $tmp_file_CPU))
    echo "greedycol"
    mpirun -mca io ^ompio -np $NP ~/numawrap  ../MG-CFD-app-OP2/bin/mgcfd_mpi_genseq_reprcolor -i input-mgcfd.dat -m ptscotch OP_PART_SIZE=2048 -op_repro_greedy_coloring 2>&1 | tee -a $greedycolfile > $tmp_file_CPU
    arr_mgcfd_greedycol+=($(calc_runtime $tmp_file_CPU))
    echo "distcol"
    mpirun -mca io ^ompio -np $NP ~/numawrap  ../MG-CFD-app-OP2/bin/mgcfd_mpi_genseq_reprcolor -i input-mgcfd.dat -m ptscotch OP_PART_SIZE=2048 2>&1 | tee -a $distfile  > $tmp_file_CPU
    arr_mgcfd_distrcol+=($(calc_runtime $tmp_file_CPU))
    echo "trivcol"
    mpirun -mca io ^ompio -np $NP ~/numawrap  ../MG-CFD-app-OP2/bin/mgcfd_mpi_genseq_trivcolor -i input-mgcfd.dat -m ptscotch OP_PART_SIZE=2048 2>&1 | tee -a $trivfile  > $tmp_file_CPU
    arr_mgcfd_trivcol+=($(calc_runtime $tmp_file_CPU))
    
done

#print results
echo -e "======= Max total runtimes ====== \n nonrepro ; temparray ; greedycol ; distrcol ; trivcolor\n"

echo " Airfoil "
for index in "${!arr_airfoil_nonrepro[@]}"; 
do
    echo "${arr_airfoil_nonrepro[$index]};${arr_airfoil_temparray[$index]};${arr_airfoil_greedycol[$index]};${arr_airfoil_distrcol[$index]};${arr_airfoil_trivcol[$index]}"; 
done

echo -e "\n\n Aero "
for index in "${!arr_aero_nonrepro[@]}"; 
do
    echo "${arr_aero_nonrepro[$index]};${arr_aero_temparray[$index]};${arr_aero_greedycol[$index]};${arr_aero_distrcol[$index]};${arr_aero_trivcol[$index]}"; 
done

echo -e "\n\n Mg-cfd "
for index in "${!arr_mgcfd_nonrepro[@]}"; 
do
    echo "${arr_mgcfd_nonrepro[$index]};${arr_mgcfd_temparray[$index]};${arr_mgcfd_greedycol[$index]};${arr_mgcfd_distrcol[$index]};${arr_mgcfd_trivcol[$index]}"; 
done


rm $tmp_file_CPU