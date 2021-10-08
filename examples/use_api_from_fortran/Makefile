# This Makefile assumes you have installed the C++ TCPB client with make install
# and added the lib and include folders to your environment

FC=gfortran
FFLAGS=-fPIC
LIBS=-L/u/cruzeiro/tcpb-cpp/lib -ltcpb

test_api: test_api.F90
	$(FC) $(FFLAGS) $(LIBS) -o $@ $<

.PHONY: clean
clean:
	@rm -v test_api
