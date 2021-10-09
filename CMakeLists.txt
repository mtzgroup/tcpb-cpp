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

	option(INSTALL_TESTS "Compile tests in the tests folder of the install directory" FALSE)
else()
	set(INSTALL_HEADERS TRUE)
	set(INSTALL_TESTS FALSE)
endif()

# on Windows, make MSVC auto-create import libraries just like MinGW does
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

#add_subdirectory(src)
if (INSTALL_TESTS)
  add_subdirectory(test)
endif()

#--------------------------------------------------------------
if(NOT INSIDE_AMBER)
	print_build_report()
endif()
