# This Makefile assumes you have installed the C++ TCPB client with make install
# and added the lib and include folders to your environment

include ../../../config.h

LIBS=-L$(LIBDIR) -lprotobuf -ltcpb

test_api_cpp_openmm: test_api.cpp
	$(CXX) $(TCPB_CXXFLAGS) -o $@ $< -I$(INCDIR) $(LIBS)

.PHONY: clean
clean:
	@rm -v test_api_cpp
