#!/bin/bash

# docker run -it -v `pwd`:/tmp/tcpb-cpp -w /tmp/tcpb-cpp sseritan/tcpb-client-docker:latest # Interactive version
docker run -v `pwd`:/tmp/tcpb-cpp -w /tmp/tcpb-cpp sseritan/tcpb-client-docker:latest ./run_pipeline_tests.sh
