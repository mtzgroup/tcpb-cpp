cmake_minimum_required(VERSION 3.1) #need 3.1 for "cmake -E env" command
project(tcpb NONE)

#version number
#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
set(tcpb_MAJOR_VERSION 1)
set(tcpb_MINOR_VERSION 0)
set(tcpb_TWEAK_VERSION 0)

set(tcpb_VERSION "${tcpb_MAJOR_VERSION}.${tcpb_MINOR_VERSION}.${tcpb_TWEAK_VERSION}")

if(NOT INSIDE_AMBER)
	# initialization and include paths
	#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
	include(cmake/AmberBuildSystemInit.cmake)

	#now enable the languages
	enable_language(C CXX)

	include(AmberBuildSystem2ndInit)

	set(LINALG_LIBS_REQUIRED FALSE)
	set(NEEDED_3RDPARTY_TOOLS protobuf)
	set(REQUIRED_3RDPARTY_TOOLS )
	include(3rdPartyTools)

	include(CompilerFlags)

	#CPack setup
	# --------------------------------------------------------------------
	set(PACKAGE_NAME "tcpb")
	set(PACKAGE_FILENAME "tcpb")

	set(BUNDLE_IDENTIFIER org.martinezlab.tcpb)
	set(BUNDLE_SIGNATURE TCPB)
	include(Packaging)

	# build options
	option(INSTALL_HEADERS "Copy headers to the include/tcpb folder of the install directory." TRUE)

	option(INSTALL_TESTS "Compile tests in the tests folder of the install directory" TRUE)

	# Ensure that protobuf will be read from the environment
	string(REPLACE ":" ";" _lib_path "$ENV{LD_LIBRARY_PATH}")
	set( CMAKE_LIBRARY_PATH ${_lib_path} )

	string(REPLACE ":" ";" _inc_path "$ENV{CPATH}")
	set( CMAKE_INCLUDE_PATH ${_inc_path} )
else()
	set(INSTALL_HEADERS TRUE)
	set(INSTALL_TESTS FALSE)
endif()

# Ensure that protobuf library, include folder, and executable are found
find_library(PROTOBUF_LIB protobuf)
if(NOT PROTOBUF_LIB)
    message(FATAL_ERROR "The protobuf library was not found!")
endif()
find_path(PROTOBUF_INCLUDES google)
if(NOT PROTOBUF_INCLUDES)
    message(FATAL_ERROR "The protobuf include folder was not found!")
endif()
find_program(PROTOBUF_PROGRAM protoc)
if(NOT PROTOBUF_PROGRAM)
    message(FATAL_ERROR "The program protoc was not found!")
endif()

# Ensure that Threads is in the environment
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Run protoc to generate CPP and H files from the proto file and place them on the src folder
# Note: CMAKE_CURRENT_SOURCE_DIR is the location of this CMakeLists.txt file
#       CMAKE_CURRENT_BINARY_DIR is the directory where I am building from
execute_process(COMMAND ${PROTOBUF_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/proto/terachem_server.proto --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto --cpp_out=.)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)
file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/terachem_server.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/src/terachem_server.pb.cpp)
file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/terachem_server.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/src/terachem_server.pb.h)

# on Windows, make MSVC auto-create import libraries just like MinGW does
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

add_subdirectory(src)
if (INSTALL_TESTS)
  add_subdirectory(tests)
endif()

#--------------------------------------------------------------
if(NOT INSIDE_AMBER)
	print_build_report()
endif()
