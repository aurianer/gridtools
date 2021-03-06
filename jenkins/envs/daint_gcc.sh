#!/bin/bash

source $(dirname "$BASH_SOURCE")/daint.sh

module swap PrgEnv-cray PrgEnv-gnu
module swap gcc/7.3.0

export CXX=$(which CC)
export CC=$(which cc)
export FC=$(which ftn)

export GTCMAKE_GT_ENABLE_BACKEND_CUDA='OFF'

export GTCMAKE_CMAKE_CXX_FLAGS='-march=haswell'
export GTCMAKE_CMAKE_CXX_FLAGS_RELEASE='-Ofast -DNDEBUG'
