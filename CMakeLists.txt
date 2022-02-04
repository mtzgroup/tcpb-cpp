cmake_minimum_required(VERSION 3.8.0) #need 3.8 for FindProtobuf.cmake
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
	enable_language(C CXX Fortran)

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

	option(INSTALL_EXAMPLES "Compile examples in the exemples folder of the install directory" TRUE)

	option(BUILD_PYTHON "Install pytcpb in the Python environment" TRUE)

	# Ensure that protobuf will be read from the environment
	string(REPLACE ":" ";" _lib_path "$ENV{LD_LIBRARY_PATH}")
	set( CMAKE_LIBRARY_PATH ${_lib_path} )

	string(REPLACE ":" ";" _inc_path "$ENV{CPATH}")
	set( CMAKE_INCLUDE_PATH ${_inc_path} )

	# Locate the protobuf library
	find_package(Protobuf)
	if(NOT Protobuf_FOUND)
         message(FATAL_ERROR "Protobuf was not found!")
    endif()
else()
	set(INSTALL_HEADERS TRUE)
	set(INSTALL_TESTS FALSE)
	set(INSTALL_EXAMPLES FALSE)
endif()

# Ensure that Threads is in the environment
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# on Windows, make MSVC auto-create import libraries just like MinGW does
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

add_subdirectory(src)
if (BUILD_PYTHON AND NOT INSIDE_AMBER)
  if (NOT DEFINED PYTHON_EXECUTABLE)
    set(PYTHON_EXECUTABLE "python")
  endif()
  add_subdirectory(pytcpb)
endif()
if (INSTALL_TESTS)
  add_subdirectory(tests)
endif()
if (INSTALL_EXAMPLES)
  add_subdirectory(examples)
  add_subdirectory(api_examples)
endif()

#--------------------------------------------------------------
if(NOT INSIDE_AMBER)
	print_build_report()
endif()
