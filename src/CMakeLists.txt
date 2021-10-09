#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Parse tcpbfiles (this file contains a list of all files to be compiled and the respective objects)

# read each non-empty line into an element of a list
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/tcpbfiles TCPBFILES_CONTENTS)

# get rid of backslashes
string(REPLACE "\\" "" TCPBFILES_CONTENTS "${TCPBFILES_CONTENTS}")

# name of list that we are curreently appending to
set(LIST_NAME "")

foreach(LINE ${TCPBFILES_CONTENTS})

	# ignore comment lines
	if(NOT "${LINE}" MATCHES "^#")

		# extract the name of the source file mentioned in the line (a string after whitespace or an equals sign)
		string(REGEX MATCH "[^ :=]+\.(o|cpp|c|LIBTCPB\.o)" SOURCE_FILE_NAME "${LINE}")

		# get name of variable that the following list is being set to
		# must exclude parentheses so that we don't match dereferences of other variables
		string(REGEX MATCH "[^$\(\)]+=" VARIABLE_NAME "${LINE}")

		# if we are starting a new source list, update LIST_NAME accordingly
		if(NOT "${VARIABLE_NAME}" STREQUAL "")
			string(REPLACE "=" "" VARIABLE_NAME "${VARIABLE_NAME}")
			set(LIST_NAME ${VARIABLE_NAME})
		endif()

		# did we get a new source file?
		if(NOT "${SOURCE_FILE_NAME}" STREQUAL "")

			if("${LIST_NAME}" STREQUAL "")
				message(FATAL_ERROR "tcpbfiles parser error: got source files before any source lists!")
			endif()

			# get rid of LIBTCPB.o suffix if it exists
			string(REPLACE "LIBTCPB.o" "cpp" SOURCE_FILE_NAME "${SOURCE_FILE_NAME}")

			# also convert .o to .cpp (used in some variables)
			string(REPLACE ".o" ".cpp" SOURCE_FILE_NAME "${SOURCE_FILE_NAME}")


			list(APPEND ${LIST_NAME} ${SOURCE_FILE_NAME})
		endif()

		#message("\"${LINE}\" - SFN: \"${SOURCE_FILE_NAME}\" - VN: \"${VARIABLE_NAME}\"")
	endif()
endforeach()

# The above loop will create the folowing variables:
# COMMON_SOURCES - all C++ source files used for both tcpb and libtcpb, that are compiled the same way for both
# CSOURCES - all C source files used for tcpb and libtcpb
# SOURCES - C++ sources for tcpb only
# LIBTCPB_OBJECTS - C++ sources for libtcpb that should be compiled with the LIBTCPB definition
# LIBTCPB_CORE_OBJECTS - C++ sources which contain "core" functionality.
# LIBTCPB_FILE_OBJECTS - C++ sources which contain basic file-related functionality. Requires core library
# LIBTCPB_TRAJ_OBJECTS - C++ sources which contain trajectory file functionality. Requires core and file libraries, as well as libxdrfile.
# LIBTCPB_PARM_OBJECTS - C++ sources which contain parameter file functionality. Requires core and file libraries.

# pub_fft.F90 is not in the source lists
set(PUBFFT_FORTRAN_SOURCE pub_fft.F90)

#---------------------------------------------------------------------------------------------------------------------------------------------------------------------

# with icc, tcpb needs -fp-model source in order to produce floating point results that match gcc
set(FP_SOURCE_FLAG "")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	check_cxx_compiler_flag("-fp-model source" FP_MODEL_SOURCE_WORKS)

	if(FP_MODEL_SOURCE_WORKS)
		set(FP_SOURCE_FLAG "-fp-model source")
	endif()
endif()

# all sources should use optimized compile flags
set_property(SOURCE ${PUBFFT_FORTRAN_SOURCE} PROPERTY COMPILE_FLAGS "${OPT_FFLAGS_SPC}")
set_property(SOURCE ${COMMON_SOURCES} ${SOURCES} ${LIBTCPB_OBJECTS} PROPERTY COMPILE_FLAGS "${OPT_CXXFLAGS_SPC} ${FP_SOURCE_FLAG}")
set_property(SOURCE ${CSOURCES} PROPERTY COMPILE_FLAGS "${OPT_CFLAGS_SPC}")

include_directories(${AMBERTOOLS_INC_DIR})

if(fftw_ENABLED)
	set_property(SOURCE PubFFT.cpp PROPERTY COMPILE_DEFINITIONS FFTW_FFT)
endif()

#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
#add the common object library

#concatenate all the source files
set(TCPB_COMMON_SOURCES ${COMMON_SOURCES} ${CSOURCES})

if(fftw_DISABLED)
	# we only need pubfft if we don't have FFTW
	list(APPEND TCPB_COMMON_SOURCES ${PUBFFT_FORTRAN_SOURCE})
endif()

add_library(tcpb_common_obj OBJECT ${TCPB_COMMON_SOURCES})
make_pic_if_needed(tcpb_common_obj)

#normally this would be applied by target_link_libraries, but since we use that intermediary object library, we have to apply it manually


#-------------------------------------------------------------------------------------
# Run protoc to generate CPP and H files from the proto file

execute_process(COMMAND protoc proto/terachem_server.proto --proto_path=proto --cpp_out=.)
execute_process(COMMAND mv terachem_server.pb.cc src/terachem_server.pb.cpp)
execute_process(COMMAND mv terachem_server.pb.h src/terachem_server.pb.h)


# NOTE: there is a CMake bug where if we were to set these as a directory-scope includes, the CUDA build would fail on some platforms with old versions of CMake
# it turns out that CMake's cuda library passes the include paths after the first one from each of these generator expressions to nvcc without the -I flag
# This causes the error "A single input file is required for a non-link phase when an outputfile is specified"
target_include_directories(tcpb_common_obj PRIVATE $<TARGET_PROPERTY:xdrfile,INTERFACE_INCLUDE_DIRECTORIES> $<TARGET_PROPERTY:netcdf,INTERFACE_INCLUDE_DIRECTORIES>)

#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
# tcpb executable

add_executable(tcpb $<TARGET_OBJECTS:tcpb_common_obj> ${SOURCES})

target_link_libraries(tcpb netcdf netlib xdrfile)

install(TARGETS tcpb DESTINATION ${BINDIR})
#------------------------------------------------------------------------------------------
# libtcpb library

add_library(libtcpb $<TARGET_OBJECTS:tcpb_common_obj> ${LIBTCPB_OBJECTS})
set_property(TARGET libtcpb PROPERTY COMPILE_DEFINITIONS LIBTCPB)

target_link_libraries(libtcpb netlib netcdf xdrfile)
remove_prefix(libtcpb)
install_libraries(libtcpb)

#tell others where to find the tcpb includes
target_include_directories(libtcpb INTERFACE .)

#------------------------------------------------------------------------------------------
# Static libraries
# (not installed, used by other targets inside Amber which only need a subset of tcpb functionality)
add_library(tcpb_core STATIC ${LIBTCPB_CORE_OBJECTS})
add_library(tcpb_file STATIC ${LIBTCPB_FILE_OBJECTS})
add_library(tcpb_traj STATIC ${LIBTCPB_TRAJ_OBJECTS})
add_library(tcpb_parm STATIC ${LIBTCPB_PARM_OBJECTS})

# all libraries include the current dir as an interface directory
targets_include_directories(tcpb_core tcpb_file tcpb_traj tcpb_parm DIRECTORIES INTERFACE .)

# tcpb_traj needs xdrfile and netcdf
target_include_directories(tcpb_traj PRIVATE $<TARGET_PROPERTY:xdrfile,INTERFACE_INCLUDE_DIRECTORIES> $<TARGET_PROPERTY:netcdf,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(tcpb_traj xdrfile netcdf)

# all libs need netlib
targets_link_libraries(tcpb_core tcpb_file tcpb_traj tcpb_parm LIBRARIES netlib)

#------------------------------------------------------------------------------------------
# DLL exports/imports

if(SHARED)
	# CMake automatically sets up the libtcpb_EXPORTS definition for libtcpb.
	# We just have to apply it to the common obj as well.
	target_compile_definitions(tcpb_common_obj PRIVATE libtcpb_EXPORTS)

	# now set up the definition for other people to use
	target_compile_definitions(libtcpb INTERFACE TCPB_USE_DLL)
endif()

#------------------------------------------------------------------------------------------
# Header installation

if(INSTALL_HEADERS)
	# grab all .h files from the main directory.
	file(GLOB TCPB_HEADERS "*.h")
	list(REMOVE_ITEM TCPB_HEADERS "SymbolExporting.h")

	# also grab xdrfile headers since some of them are used by tcpb headers
	file(GLOB XDRFILE_HEADERS "xdrfile/*.h")

	install(FILES ${TCPB_HEADERS} DESTINATION ${INCDIR}/tcpb)
	install(FILES ${XDRFILE_HEADERS} DESTINATION ${INCDIR}/tcpb/xdrfile)

	# configure SymbolExporting.h specially for the current install type
	if(SHARED)
		set(TCPB_IS_SHARED 1)
	else()
		set(TCPB_IS_SHARED 0)
	endif()
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SymbolExporting-installversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/SymbolExporting-installversion.h @ONLY)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SymbolExporting-installversion.h DESTINATION ${INCDIR}/tcpb RENAME SymbolExporting.h)

endif()

#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
#Deal with external libraries

# NOTE: you CANNOT set any directory-scope include directories that use generator expressions here.
# These expressions get propagated down into the cuda_kernels subdir, and trigger an undocumented bug in old versions of CMake
# where they get passed to nvcc without a -I prefix, breaking everything.  So please don't do it.

if(libbz2_ENABLED)
	add_definitions(-DHASBZ2)
	include_directories(${BZIP2_INCLUDE_DIR})
	targets_link_libraries(tcpb tcpb_file libtcpb LIBRARIES BZip2::BZip2)
endif()

if(zlib_ENABLED)
	add_definitions(-DHASGZ)
	include_directories(${ZLIB_INCLUDE_DIRS})
	targets_link_libraries(tcpb tcpb_file libtcpb LIBRARIES ZLIB::ZLIB)
endif()

if(fftw_ENABLED)
	target_include_directories(tcpb_common_obj PRIVATE $<TARGET_PROPERTY:fftw,INTERFACE_INCLUDE_DIRECTORIES>)
	targets_link_libraries(tcpb libtcpb LIBRARIES fftw)
endif()

#readline
if(readline_ENABLED)
	targets_link_libraries(tcpb libtcpb LIBRARIES readline)
else()
	target_compile_definitions(tcpb PRIVATE NO_READLINE)
	target_compile_definitions(libtcpb PRIVATE NO_READLINE)
endif()

if(tng_io_ENABLED)
	add_definitions(-DHAS_TNGFILE)
	include_directories($<TARGET_PROPERTY:tng_io,INTERFACE_INCLUDE_DIRECTORIES>)
	targets_link_libraries(tcpb tcpb_file libtcpb LIBRARIES tng_io)
endif()

# libsander
if(INSIDE_AMBER AND ("${AMBER_TOOLS}" MATCHES "sander" AND BUILD_SANDER_API))
	#add the sander-specific definitions and libraries
	target_compile_definitions(tcpb PRIVATE USE_SANDERLIB)
	target_link_libraries(tcpb libsander)
endif()
