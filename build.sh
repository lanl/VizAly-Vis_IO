#!/bin/bash

pushd genericio
make -j
popd

cd DataGenerator
source compile.sh
