#################
## DIRECTORIES ##
#################
SRCDIR=./src
PROTODIR=./proto
BUILDDIR=./build

TCPBSRC := 	$(SRCDIR)/exceptions.cpp \
		$(SRCDIR)/client.cpp \
		$(SRCDIR)/input.cpp \
		$(SRCDIR)/output.cpp \
		$(SRCDIR)/socket.cpp \
		$(SRCDIR)/terachem_server.pb.cpp \
		$(SRCDIR)/utils.cpp

OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(TCPBSRC))

###############
## COMPILERS ##
###############
CXX=g++
CXXFLAGS=-fPIC -std=c++11 -g -DSOCKETLOGS
PROTOC=protoc
LIBS=-lprotobuf

##################
## INSTALLATION ##
##################
VER=0.4.0
#PREFIX=/global/user_software/tcpb-client/$(VER)
PREFIX=/home/sseritan/personal_modules/software/tcpb-cpp
LIBPREFIX=$(PREFIX)/lib
INCPREFIX=$(PREFIX)/include/tcpb

################
## MAKE RULES ##
################
.PHONY: all clean install uninstall
all: $(SRCDIR)/terachem_server.pb.cpp $(BUILDDIR)/libtcpb.so.$(VER)

clean:
	@rm -rf $(BUILDDIR)
	@rm -f $(SRCDIR)/terachem_server.pb.*

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

###################
## COMPILE RULES ##
###################
$(BUILDDIR)/libtcpb.so.$(VER): $(OBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(SRCDIR)/terachem_server.pb.cpp: $(PROTODIR)/terachem_server.proto
	$(PROTOC) $< --proto_path=$(PROTODIR) --cpp_out=.
	@mv terachem_server.pb.cc $(SRCDIR)/terachem_server.pb.cpp
	@mv terachem_server.pb.h $(SRCDIR)
