include config.h

.NOTPARALLEL:clean install all
.PHONY: test pytcpb

LIBSRC := src/exceptions.cpp \
	src/client.cpp \
	src/input.cpp \
	src/output.cpp \
	src/socket.cpp \
	src/terachem_server.pb.cpp \
	src/utils.cpp \
	src/api.cpp

LIBOBJS := $(patsubst src/%.cpp, src/%.o, $(LIBSRC))
       
LIBNAME  = libtcpb

all: src/terachem_server.pb.cpp $(LIBNAME).so

$(LIBNAME).so: $(LIBOBJS)
	@echo "[TCPB]  CXX $@"
	$(VB)$(CXX) $(TCPB_CXXFLAGS) -shared -o $(LIBNAME).so $(LIBOBJS) -L$(LIBDIR) $(TCPB_LDFLAGS)

install: src/terachem_server.pb.cpp $(LIBNAME).so
	@mkdir -p $(LIBDIR)
	/bin/mv $(LIBNAME).so $(LIBDIR)
	@mkdir -p $(INCDIR)/tcpb
	@cp -v src/*.h $(INCDIR)/tcpb

uninstall:
	/bin/rm -Rf "$(INCDIR)/tcpb" "$(LIBDIR)/$(LIBNAME).so" "config.h"

.SUFFIXES: .F90 .cpp .o

.F90.o:
	@echo "[TCPB]  FC $<"
	$(VB)$(FC) $(FCFLAGS) -c $*.F90 -o $*.o

.cpp.o:
	@echo "[TCPB]  CXX $<"
	$(VB)$(CXX) $(TCPB_CXXFLAGS) -c $*.cpp -o $*.o -I$(INCDIR)

src/terachem_server.pb.cpp: proto/terachem_server.proto
	@echo "[TCPB]  PROTOC $<"
	$(PROTOC) $< --proto_path=proto --cpp_out=.
	@mv terachem_server.pb.cc src/terachem_server.pb.cpp
	@mv terachem_server.pb.h src

clean:
	/bin/rm -f $(LIBOBJS)

example:
	@cd examples/qm && make
	@cd examples/qmmm && make
	@cd examples/api/fortran && make
	@cd examples/api/fortran_openmm && make
	@cd examples/api/cpp && make
	@cd examples/api/cpp_openmm && make

pytcpb:
	@echo "[pyTCPB]  Installing pyTCPB"
	@cd pytcpb && python setup.py install
