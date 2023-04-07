#!/bin/bash

module load gcc/10.2
module load cuda/11.5
module load cmake/3.23
module refresh

######## User Configurations ########
Kokkos_HOME=$HOME/kokkos/kokkos
ADIOS2_HOME=$(pwd)
BUILD_DIR=${ADIOS2_HOME}/build-kokkos-summit
INSTALL_DIR=${ADIOS2_HOME}/install-kokkos-summit

num_build_procs=4

######## Kokkos ########
mkdir -p "${BUILD_DIR}/kokkos"
rm -f "${BUILD_DIR}/kokkos/CMakeCache.txt"
rm -rf "${BUILD_DIR}/kokkos/CMakeFiles"

ARGS=(
    -D CMAKE_BUILD_TYPE=RelWithDebInfo
    -D CMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
	-D CMAKE_CXX_COMPILER="${Kokkos_HOME}/bin/nvcc_wrapper"

    -D Kokkos_ENABLE_SERIAL=ON
    -D Kokkos_ARCH_POWER9=ON
    -D Kokkos_ENABLE_CUDA=ON
    -D Kokkos_ENABLE_CUDA_LAMBDA=ON # from Kokkos 4.0 this is not needed
    -D Kokkos_ARCH_VOLTA70=ON

    -D CMAKE_CXX_STANDARD=17
    -D CMAKE_CXX_EXTENSIONS=OFF
    -D CMAKE_POSITION_INDEPENDENT_CODE=TRUE
	-D BUILD_SHARED_LIBS=ON
)
cmake "${ARGS[@]}" -S "${Kokkos_HOME}" -B "${BUILD_DIR}/kokkos"
cmake --build "${BUILD_DIR}/kokkos" -j${num_build_procs}
cmake --install "${BUILD_DIR}/kokkos"

######## ADIOS2 ########
mkdir -p "${BUILD_DIR}/adios2"
rm -f "${BUILD_DIR}/adios2/CMakeCache.txt"
rm -rf "${BUILD_DIR}/adios2/CMakeFiles"

ARGS_ADIOS=(
    -D CMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
    -D BUILD_TESTING=OFF
    -D ADIOS2_BUILD_EXAMPLES=OFF
    -D CMAKE_CXX_COMPILER=g++
    -D CMAKE_C_COMPILER=gcc

    -D ADIOS2_USE_Kokkos=ON
    -D Kokkos_ROOT="${INSTALL_DIR}"

    -D CMAKE_POSITION_INDEPENDENT_CODE=TRUE
    -D BUILD_SHARED_LIBS=ON
)
cmake "${ARGS_ADIOS[@]}" -S "${ADIOS2_HOME}" -B "${BUILD_DIR}"/adios2
cmake --build "${BUILD_DIR}/adios2" -j${num_build_procs}
cmake --install "${BUILD_DIR}/adios2"
