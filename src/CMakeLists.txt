#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Parse tcpbfiles (this file contains a list of all files to be compiled and the respective objects)

# read each non-empty line into an element of a list
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/tcpbfiles TCPBFILES_CONTENTS)

# get rid of backslashes
string(REPLACE "\\" "" TCPBFILES_CONTENTS "${TCPBFILES_CONTENTS}")

# name of list that we are currently appending to
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

# The above loop will create the following variables:
# SOURCES - C++ sources for libtcpb
# OBJECTS - C++ sources for libtcpb

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
set_property(SOURCE ${SOURCES} ${OBJECTS} PROPERTY COMPILE_FLAGS "${OPT_CXXFLAGS_SPC} ${OPT_CFLAGS_SPC} ${FP_SOURCE_FLAG}")

if(INSIDE_AMBER)
  include_directories(${AMBERTOOLS_INC_DIR})
else()
  include_directories(${INCDIR})
endif()

#---------------------------------------------------------------------------------------------------------------------------------------------------------------------
#add the common object library

add_library(tcpb_common_obj OBJECT ${SOURCES})
make_pic_if_needed(tcpb_common_obj)

#normally this would be applied by target_link_libraries, but since we use that intermediary object library, we have to apply it manually

#------------------------------------------------------------------------------------------
# libtcpb library

add_library(libtcpb $<TARGET_OBJECTS:tcpb_common_obj> ${OBJECTS})
set_property(TARGET libtcpb PROPERTY COMPILE_DEFINITIONS LIBTCPB)

target_link_libraries(libtcpb PUBLIC protobuf PRIVATE "${PROTOBUF_LIB}" Threads::Threads)
target_include_directories(libtcpb PRIVATE "${PROTOBUF_INCLUDES}")
remove_prefix(libtcpb)
install_libraries(libtcpb)

#tell others where to find the tcpb includes
target_include_directories(libtcpb INTERFACE .)

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

	install(FILES ${TCPB_HEADERS} DESTINATION ${INCDIR}/tcpb)

	# configure SymbolExporting.h specially for the current install type
	if(SHARED)
		set(TCPB_IS_SHARED 1)
	else()
		set(TCPB_IS_SHARED 0)
	endif()

endif()
