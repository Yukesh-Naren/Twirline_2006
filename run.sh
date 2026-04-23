#!/bin/bash

if[ $# -lt 1 ]; then 
    echo "Usage: ./run.sh file.frx"
    exit 1
fi

./build/ferrox "$1" || exit 1
java -jar rars.jar output.s