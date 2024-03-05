#!/bin/bash

# Inputs:
# $1: The command to run (build&test, format, or tidy)
# $2: A codecov.io upload token passed in as a secret from the project settings on Github
echo "Running DockerEntrypoint.sh"
echo "Command: $1"
apt-get install glfw3
if [ $1 = "build&test" ]
then
    cd src
    # Start a virtual display for GUI apps
    Xvfb :1 -screen 0 1920x1080x16&
    export DISPLAY=:1.0

    # Build project
    ./compile_shaders.sh
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build --config Debug

    # Required for code coverage in Clang
    export LLVM_PROFILE_FILE=/profile.profraw

    ctest --test-dir build -C Debug --output-on-failure

    # If ctest failed then return an error here
    if [ $? != 0 ]
    then
        exit 1
    fi

    # Calculate code coverage and report it to codecov.io
    llvm-profdata merge -sparse /profile.profraw -o build/coverage.profdata
    llvm-cov show build/test/UIUnitTests -instr-profile=build/coverage.profdata > build/coverage.txt
    curl -Os https://uploader.codecov.io/latest/linux/codecov
    chmod +x codecov
    ./codecov -t $2
elif [ $1 = "format" ]
then
    cd src
    clang-format --dry-run --Werror ui/*pp
    clang-format --dry-run --Werror renderer/*pp
elif [ $1 = "tidy" ]
then
    cd src
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build --config Debug
    clang-tidy -p build ui/*pp
fi

# to run locally run following commands from the directory holding this file: 
# docker build -t qttest -f Dockerfile .
# docker run --name qttest-container -v "$(pwd)"/../../../:/src qttest "build&test"
# docker run -it --name qttest-container -v "$(pwd)"/../../../:/src --entrypoint /bin/bash qttest (for interactive mode)
# to remove after running: 
# docker rm qttest-container
# docker image rm qttest
