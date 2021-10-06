# This Makefile assumes you have installed the C++ TCPB client with make install
# and added the lib and include folders to your environment

CXX=g++
CXXFLAGS=-fPIC -std=c++11 -pthread -g
LIBS=-lprotobuf -ltcpb

tcpb-example: tcpb-example.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

.PHONY: clean
clean:
	@rm -v tcpb-example
