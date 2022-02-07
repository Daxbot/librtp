# librtp

C99 RTP/RTCP library

### Build

This project uses the cmake build system. To build run the following:

    mkdir build
    cd build
    cmake ..
    make

### Test

This project uses the google test system.

    sudo apt install googletest libgtest-dev
    make && make test

### Examples

This project includes Linux (PulseAudio) example applications.  To build
project examples run the following:

    sudo apt install libopus-dev libpulse-dev
    cmake -DBUILD_EXAMPLES=ON ..
    make

### Documentation

This project uses the Doxygen documentation engine. To build documentation run
the following:

    sudo apt install doxygen
    cmake -DBUILD_DOCS=ON ..
    make
