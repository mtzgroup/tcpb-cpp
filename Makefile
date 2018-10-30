#################
## DIRECTORIES ##
#################
SRCDIR=./src
INCDIR=./include
PROTODIR=./proto
BUILDDIR=./build
OBJDIR=$(BUILDDIR)/obj

###############
## COMPILERS ##
###############
CXX=g++
CXXFLAGS=-I$(INCDIR) -fPIC
PROTOC=protoc
LIBS=-lprotobuf

##################
## INSTALLATION ##
##################
VER=0.3.0
#PREFIX=/global/user_software/tcpb-client/$(VER)
PREFIX=/home/sseritan/personal_modules/software/tcpb-cpp

################
## MAKE RULES ##
################
.PHONY: all clean install uninstall
all: libtcpb.so.$(VER)

clean:
	@rm -rf $(OBJDIR) $(BUILDDIR)
	@rm -f $(SRCDIR)/terachem_server.pb.cpp $(INCDIR)/terachem_server.pb.h

install:
	@echo "Installing TCPB C++ client into $(PREFIX)"
	@mkdir -p $(PREFIX)/lib
	@cp -v $(BUILDDIR)/libtcpb.so.$(VER) $(PREFIX)/lib
	@ln -sfn $(PREFIX)/lib/libtcpb.so.$(VER) $(PREFIX)/lib/libtcpb.so
	@mkdir -p $(PREFIX)/include
	@cp -v $(INCDIR)/tcpb.h $(PREFIX)/include
	@cp -v $(INCDIR)/terachem_server.pb.h $(PREFIX)/include

uninstall:
	@echo "Uninstalling TCPB C++ client from $(PREFIX)"
	@rm -v $(PREFIX)/lib/{libtcpb.so.$(VER),libtcpb.so}
	@rm -v $(PREFIX)/include/tcpb.h
	@rm -v $(PREFIX)/include/terachem_server.pb.h

###########
## RULES ##
###########
$(SRCDIR)/terachem_server.pb.cpp $(INCDIR)/terachem_server.pb.h: $(PROTODIR)/terachem_server.proto
	$(PROTOC) $< --proto_path=$(PROTODIR) --cpp_out=.
	@mv terachem_server.pb.cc $(SRCDIR)/terachem_server.pb.cpp
	@mv terachem_server.pb.h $(INCDIR)

$(OBJDIR)/terachem_server.pb.o: $(SRCDIR)/terachem_server.pb.cpp $(INCDIR)/terachem_server.pb.h
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/tcpb.o: $(SRCDIR)/tcpb.cpp $(INCDIR)/tcpb.h $(INCDIR)/terachem_server.pb.h
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<


libtcpb.so.$(VER): $(OBJDIR)/tcpb.o $(OBJDIR)/terachem_server.pb.o
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -shared -o $(BUILDDIR)/$@ $^ $(LIBS)


