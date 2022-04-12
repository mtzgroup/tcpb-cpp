# This Makefile assumes you have installed the C++ TCPB client with make install
# and added the lib and include folders to your environment

include ../../../config.h

LIBS=-L$(LIBDIR) -ltcpb

test_api_fortran: test_api.F90
	$(FC) $(FFLAGS) $(LIBS) -o $@ $<

.PHONY: clean
clean:
	@rm -v test_api_fortran
