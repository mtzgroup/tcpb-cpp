include config.h

.NOTPARALLEL:clean install all
.PHONY: test

LIBSRC := src/exceptions.cpp \
	src/client.cpp \
	src/input.cpp \
	src/output.cpp \
	src/server.cpp \
	src/socket.cpp \
	src/terachem_server.pb.cpp \
	src/utils.cpp \
	src/api.cpp

LIBOBJS := $(patsubst src/%.cpp, src/%.o, $(LIBSRC))
       
LIBNAME  = libtcpb

TESTSRC := 	tests/input_test.cpp \
		tests/socket_test.cpp \
		tests/tcpb_test.cpp

TESTBIN := $(patsubst tests/%.cpp, tests/%, $(TESTSRC))

all: src/terachem_server.pb.cpp $(LIBNAME).so

$(LIBNAME).so: $(LIBOBJS)
	@echo "[TCPB]  CXX $@"
	$(VB)$(CXX) -shared -o $(LIBNAME).so $(LIBOBJS) $(TCPB_LDFLAGS)

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
	$(VB)$(CXX) $(TCPB_CXXFLAGS) -c $*.cpp -o $*.o

src/terachem_server.pb.cpp: proto/terachem_server.proto
	@echo "[TCPB]  PROTOC $<"
	$(PROTOC) $< --proto_path=proto --cpp_out=.
	@mv terachem_server.pb.cc src/terachem_server.pb.cpp
	@mv terachem_server.pb.h src

clean:
	/bin/rm -f $(LIBOBJS)

ifdef TESTDIR
test: $(TESTBIN) $(LIBDIR)/$(LIBNAME).so
	@echo "TCPB: Running input_test"
	@cd tests && ./input_test
	@echo "TCPB: Running socket_test"
	@cd tests && ./socket_test
	@echo "TCPB: Running tcpb_test"
	@cd tests && ./tcpb_test

test-clean:
	/bin/rm -f $(TESTBIN)

tests/%: tests/%.cpp
	$(CXX) $(TCPB_CXXFLAGS) -o $@ $< $(TESTS_LDFLAGS) -I$(INCDIR) -L$(LIBDIR)
endif