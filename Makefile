#################
## DIRECTORIES ##
#################
SRCDIR=./src
PROTODIR=./proto
BUILDDIR=./build

TCPBSRC := 	$(SRCDIR)/tcpb.cpp \
		$(SRCDIR)/terachem_server.pb.cpp \
		$(SRCDIR)/utils.cpp

TCPBOBJ := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(TCPBSRC))

###############
## COMPILERS ##
###############
CXX=g++
CXXFLAGS=-fPIC -std=c++11
#CXXFLAGS=-fPIC -std=c++11 -g -DSOCKETLOGS
PROTOC=protoc
LIBS=-lprotobuf

##################
## INSTALLATION ##
##################
VER=2.0.0
#PREFIX=/global/user_software/tcpb-client/$(VER)
PREFIX=/home/sseritan/personal_modules/software/tcpb-cpp

################
## MAKE RULES ##
################
.PHONY: all clean install uninstall
all: $(SRCDIR)/terachem_server.pb.cpp $(BUILDDIR)/libtcpb.so.$(VER)

clean:
	@rm -rf $(BUILDDIR)
	@rm -f $(SRCDIR)/terachem_server.pb.cpp $(SRCDIR)/terachem_server.pb.h

install:
	@echo "Installing TCPB C++ client into $(PREFIX)"
	@mkdir -p $(PREFIX)/lib
	@cp -v $(BUILDDIR)/libtcpb.so.$(VER) $(PREFIX)/lib
	@ln -sfn $(PREFIX)/lib/libtcpb.so.$(VER) $(PREFIX)/lib/libtcpb.so
	@mkdir -p $(PREFIX)/include
	@cp -v $(SRCDIR)/{tcpb.h,terachem_server.pb.h} $(PREFIX)/include

uninstall:
	@echo "Uninstalling TCPB C++ client from $(PREFIX)"
	@rm -v $(PREFIX)/lib/{libtcpb.so.$(VER),libtcpb.so}
	@rm -v $(PREFIX)/include/tcpb.h $(PREFIX)/include/terachem_server.pb.h

###########
## RULES ##
###########
$(BUILDDIR)/libtcpb.so.$(VER): $(TCPBOBJ)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(SRCDIR)/terachem_server.pb.cpp: $(PROTODIR)/terachem_server.proto
	$(PROTOC) $< --proto_path=$(PROTODIR) --cpp_out=.
	@mv terachem_server.pb.cc $(SRCDIR)/terachem_server.pb.cpp
	@mv terachem_server.pb.h $(SRCDIR)
