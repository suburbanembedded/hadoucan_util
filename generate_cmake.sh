#!/usr/bin/env bash

trap 'exit -1' err

set -v

if [ -d build ]; then rm -rf build; fi

BASE_PATH=$PWD

mkdir -p build/debug
pushd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug $BASE_PATH
popd

mkdir -p build/relwithdebinfo
pushd build/relwithdebinfo
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo $BASE_PATH
popd

mkdir -p build/release
pushd build/release
cmake -DCMAKE_BUILD_TYPE=Release $BASE_PATH
popd
