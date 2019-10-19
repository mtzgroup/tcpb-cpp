CXX=g++
CXXFLAGS=-std=c++11 -g
LIBS=-lprotobuf -ltcpb

.PHONY: all
all: socket_test

socket_test: socket_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

.PHONY: clean
clean:
	@rm socket_test
	@rm *.log
