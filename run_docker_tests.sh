#!/bin/bash

VERSION=1.0
# Interactive version: docker run -it -v "`pwd`":/tmp/tcpb -w /home/tcpb-cpp --name tcpb --rm sseritan/tcpb-client-docker:$VERSION
docker run -v "`pwd`":/tmp/tcpb -w /home/tcpb-cpp --name tcpb --rm sseritan/tcpb-client-docker:$VERSION /bin/bash -c "cp -r /tmp/tcpb/* /home/tcpb-cpp; ./run_pipeline.sh"
