#!/bin/bash
# Designed to build and run tests inside the sseritan/tcpb-client-docker:latest container

cd proto
protoc terachem_server.proto --python_out=../tests
cd ../

sed -i 's/^PREFIX=.*/PREFIX=\/tmp\/install/' Makefile
make clean && make && make install
export LD_LIBRARY_PATH=/tmp/install/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=/tmp/install/lib:$LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/tmp/install/include:$CPLUS_INCLUDE_PATH

cd tests/
make all
python test_tcpb.py
