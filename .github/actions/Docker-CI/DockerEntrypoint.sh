#!/bin/bash

echo "Running DockerEntrypoint.sh"
echo "Command: $1"
if [ $1 = "build&test" ]
then
    cd src
    cmake -B build -S .
    make -C build
elif [ $1 = "format" ]
then
    cd src
    clang-format --dry-run --Werror ui/*pp
elif [ $1 = "tidy" ]
then
    cd src
    clang-tidy -p build ui/*pp
fi