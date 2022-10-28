#!/bin/bash

echo "Running DockerEntrypoint.sh"
echo "Command: $1"
if [ $1 = "build&test" ]
then
    cd src
    #startx&
    Xvfb :1 -screen 0 1920x1080x16&
    export DISPLAY=:1.0
    #export QT_QPA_PLATFORM=vkkhrdisplay
    #export LIBGL_ALWAYS_SOFTWARE=true
    #export GALLIUM_DRIVER=llvmpipe
    ./compile_shaders.sh
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build --config Debug

    #cd build
    ctest --test-dir build -C Debug --output-on-failure

    if [ $? != 0 ]
    then
        exit $?
    fi

    cd ..
    bash <(curl -S https://codecov.io/bash)
elif [ $1 = "format" ]
then
    cd src
    clang-format --dry-run --Werror ui/*pp
elif [ $1 = "tidy" ]
then
    cd src
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build --config Debug
    clang-tidy -p build ui/*pp
fi

# to run locally: 
# docker build -t qttest -f Dockerfile .
# docker run --name qttest-container --privileged -v "$(pwd)"/../../../:/src qttest "build&test"
# to remove after running: 
# docker rm qttest-container
# docker image rm qttest
