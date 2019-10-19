#!/bin/bash
# Designed to build and run tests inside the sseritan/tcpb-client-docker:latest container
# docker run -v \`pwd`:/tmp/tcpb-cpp -w /tmp/tcpb-cpp sseritan/tcpb-client-docker:latest ./run_pipeline_tests.sh

PWD=`pwd`

# Makefile modifications
cp Makefile Makefile.docker
sed -i 's/^BUILDDIR=.*/BUILDDIR=.\/build_docker/' Makefile.docker
sed -i 's/^PREFIX=.*/PREFIX=\/tmp\/install/' Makefile.docker

# Installation
make -f Makefile.docker clean
make -f Makefile.docker
make -f Makefile.docker install
export LD_LIBRARY_PATH=/tmp/install/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=/tmp/install/lib:$LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/tmp/install/include:$CPLUS_INCLUDE_PATH

# Tests
cd tests/
make clean && make all
./socket_test
make clean

# Clean up directories for local Docker debugging
cd ../
make -f Makefile.docker clean
rm Makefile.docker
