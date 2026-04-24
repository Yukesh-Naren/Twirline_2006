#!/bin/bash

if [ ! -d build ]; then
    mkdir build
    cd build
    cmake ..
    make
    cd ..
fi

./build/ferrox "$1" || exit 1
java -jar rars1_6.jar output.s
