# This Makefile assumes you have installed the C++ TCPB client with make install
# and added the lib and include folders to your environment

include ../../config.h

LIBS=-L$(LIBDIR) -lprotobuf -ltcpb

tcpb-example: tcpb-example.cpp
	$(CXX) $(TCPB_CXXFLAGS) -o $@ $< -I$(INCDIR) $(LIBS)

.PHONY: clean
clean:
	@rm -v tcpb-example
