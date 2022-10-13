#!/bin/bash

echo "Running DockerEntrypoint.sh"
echo "Command: $1"
if [ $1 = "build&test" ]
then
    cd src
    export QT_QPA_PLATFORM=offscreen
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --config Debug

    cd build
    ctest -C Debug

    cd ..
    bash <(curl -S https://codecov.io/bash)
elif [ $1 = "format" ]
then
    cd src
    clang-format --dry-run --Werror ui/*pp
elif [ $1 = "tidy" ]
then
    cd src
    clang-tidy -p build ui/*pp
fi

# to run locally: docker run --name qttest-container --privileged -v "$(pwd)"/../../../:/src qttest "build&test"
# to remove after running: docker rm qttest-container
