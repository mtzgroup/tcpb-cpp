# This Makefile assumes you have installed the C++ TCPB client with make install
# and added the lib and include folders to your environment

CXX=g++
CXXFLAGS=
LIBS=-lprotobuf -ltcpb

tcpb-example: tcpb-example.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

.PHONY: clean
clean:
	@rm -v tcpb-example
