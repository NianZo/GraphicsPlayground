#!/bin/bash

echo "Running DockerEntrypoint.sh"
echo "Command: $1"
if [ $1 = "build&test" ]
then
    cd src
    cmake -B build -S .
    make -C build
fi