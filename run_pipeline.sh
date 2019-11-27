#!/bin/bash
# Designed to build and run tests inside the sseritan/tcpb-client-docker container

# Makefile modifications
sed -i 's/^PREFIX=.*/PREFIX=\/tmp\/install/' Makefile

# Installation
echo "Building and installing TCPB C++ interface..."
make clean && make && make install
export LD_LIBRARY_PATH=/tmp/install/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=/tmp/install/lib:$LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/tmp/install/include:$CPLUS_INCLUDE_PATH

# Tests
echo "Running tests..."
make tests