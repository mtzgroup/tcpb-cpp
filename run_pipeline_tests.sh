#!/bin/bash
# Designed to build and run tests inside the sseritan/tcpb-client-docker:latest container
# docker run -v \`pwd`:/tmp/tcpb-cpp -w /tmp/tcpb-cpp sseritan/tcpb-client-docker:latest ./run_pipeline_tests.sh

sed -i 's/^PREFIX=.*/PREFIX=\/tmp\/install/' Makefile
make clean && make && make install
export LD_LIBRARY_PATH=/tmp/install/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=/tmp/install/lib:$LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/tmp/install/include:$CPLUS_INCLUDE_PATH

cd tests/
make all

./socket_test

# Clean up directories for local Docker debugging
cd ../
make clean
