#################
## DIRECTORIES ##
#################
SRCDIR=./src
PROTODIR=./proto
BUILDDIR=./build
TESTDIR=./tests

TCPBSRC := 	$(SRCDIR)/exceptions.cpp \
		$(SRCDIR)/client.cpp \
		$(SRCDIR)/input.cpp \
		$(SRCDIR)/output.cpp \
		$(SRCDIR)/server.cpp \
		$(SRCDIR)/socket.cpp \
		$(SRCDIR)/terachem_server.pb.cpp \
		$(SRCDIR)/utils.cpp

TCPBOBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(TCPBSRC))

TESTSRC := 	$(TESTDIR)/socket_test.cpp

TESTBIN := $(patsubst $(TESTDIR)/%.cpp, $(TESTDIR)/%, $(TESTSRC))

###############
## COMPILERS ##
###############
CXX=g++
CXXFLAGS=-fPIC -std=c++11 -pthread -g -DSOCKETLOGS
PROTOC=protoc
LIBS=-lprotobuf

TESTFLAGS=-std=c++11 -pthread -g
TESTLIBS=-lprotobuf -ltcpb

##################
## INSTALLATION ##
##################
VER=1.0.0a1
#PREFIX=/global/user_software/tcpb-client/$(VER)
PREFIX=/home/sseritan/personal_modules/software/tcpb-cpp
LIBPREFIX=$(PREFIX)/lib
INCPREFIX=$(PREFIX)/include/tcpb

################
## MAKE RULES ##
################
.PHONY: all clean install uninstall tests
all: $(SRCDIR)/terachem_server.pb.cpp $(BUILDDIR)/libtcpb.so.$(VER)

clean:
	@rm -rf $(BUILDDIR)
	@rm -f $(SRCDIR)/terachem_server.pb.*
	@rm -f $(TESTBIN) $(TESTDIR)/*.log

install:
	@echo "Installing TCPB C++ library into $(PREFIX)"
	@mkdir -p $(LIBPREFIX)
	@cp -v $(BUILDDIR)/libtcpb.so.$(VER) $(LIBPREFIX)
	@ln -sfn $(LIBPREFIX)/libtcpb.so.$(VER) $(LIBPREFIX)/libtcpb.so
	@mkdir -p $(INCPREFIX)
	@cp -v $(SRCDIR)/*.h $(INCPREFIX)

uninstall:
	@echo "Uninstalling TCPB C++ library from $(PREFIX)"
	@rm -v $(LIBPREFIX)/{libtcpb.so.$(VER),libtcpb.so}
	@rm -rv $(INCPREFIX)

tests: $(TESTBIN) $(BUILDDIR)/libtcpb.so.$(VER)
	@$(TESTDIR)/socket_test

###################
## COMPILE RULES ##
###################
$(BUILDDIR)/libtcpb.so.$(VER): $(TCPBOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(SRCDIR)/terachem_server.pb.cpp: $(PROTODIR)/terachem_server.proto
	$(PROTOC) $< --proto_path=$(PROTODIR) --cpp_out=.
	@mv terachem_server.pb.cc $(SRCDIR)/terachem_server.pb.cpp
	@mv terachem_server.pb.h $(SRCDIR)

$(TESTDIR)/%: $(TESTDIR)/%.cpp
	$(CXX) $(TESTFLAGS) -o $@ $< $(TESTLIBS)
