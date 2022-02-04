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
set_property(SOURCE ${SOURCES} PROPERTY COMPILE_FLAGS "${OPT_CXXFLAGS_SPC} ${OPT_CFLAGS_SPC} ${FP_SOURCE_FLAG}")

#-----------------------------------------------------------------------------------------------------
# Run protoc to generate CPP and header files from the proto file and place them on the src folder
# Note: CMAKE_CURRENT_SOURCE_DIR is the location of this CMakeLists.txt file
#       CMAKE_CURRENT_BINARY_DIR is the directory where I am building from
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/terachem_server.pb.cpp ${CMAKE_CURRENT_SOURCE_DIR}/terachem_server.pb.h
                   COMMAND protobuf::protoc ${CMAKE_CURRENT_SOURCE_DIR}/../proto/terachem_server.proto --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../proto --cpp_out=.
				   COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/terachem_server.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/../src/terachem_server.pb.cpp
				   COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/terachem_server.pb.h  ${CMAKE_CURRENT_SOURCE_DIR}/../src/terachem_server.pb.h
				   DEPENDS protobuf::protoc)

#------------------------------------------------------------------------------------------
# libtcpb library

add_library(libtcpb ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/terachem_server.pb.cpp ${CMAKE_CURRENT_SOURCE_DIR}/terachem_server.pb.h)

target_link_libraries(libtcpb PUBLIC protobuf::libprotobuf PRIVATE Threads::Threads)

# The following definition might be useful when linking to certain protocol buffers compilations
#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

if(Protobuf_INCLUDE_DIRS)
	target_include_directories(libtcpb PRIVATE ${Protobuf_INCLUDE_DIRS})
endif()
if(INSIDE_AMBER)
    get_target_property(libprotobuf_SOURCE_DIR protobuf::libprotobuf SOURCE_DIR)
	target_include_directories(libtcpb PRIVATE ${libprotobuf_SOURCE_DIR}/../src)
endif()
remove_prefix(libtcpb)
install_libraries(libtcpb)

# Create the headers inside the build folder, just to serve to other tools
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/tcpb)
file(GLOB TCPB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_custom_command(TARGET libtcpb POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${TCPB_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/terachem_server.pb.h" ${CMAKE_CURRENT_BINARY_DIR}/include/tcpb)
target_include_directories(libtcpb INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/include)

#------------------------------------------------------------------------------------------
# DLL exports/imports

if(SHARED)
	# now set up the definition for other people to use
	target_compile_definitions(libtcpb INTERFACE TCPB_USE_DLL)
endif()

#------------------------------------------------------------------------------------------
# Header installation

if(INSTALL_HEADERS)

    if(INSIDE_AMBER)
        install(FILES ${TCPB_HEADERS} DESTINATION ${AMBERTOOLS_INC_DIR}/tcpb)
    else()
        install(FILES ${TCPB_HEADERS} DESTINATION ${INCDIR}/tcpb)
    endif()

endif()
