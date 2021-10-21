# Copy headers file to be able to use them during compile time

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/tcpb)
file(GLOB TCPB_HEADERS "${tcpb_SOURCE_DIR}/src/*.h")
file(COPY ${TCPB_HEADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/tcpb)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# executables

add_executable(input_test input_test.cpp)
target_link_libraries(input_test PUBLIC libtcpb PRIVATE "${PROTOBUF_LIB}" Threads::Threads)
target_include_directories(input_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
install(TARGETS input_test DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)
add_custom_command(TARGET input_test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${tcpb_SOURCE_DIR}/tests/input ${CMAKE_INSTALL_PREFIX}/tests/input)

add_executable(socket_test socket_test.cpp)
target_link_libraries(socket_test PUBLIC libtcpb PRIVATE "${PROTOBUF_LIB}" Threads::Threads)
target_include_directories(socket_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
install(TARGETS socket_test DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)

add_executable(tcpb_test tcpb_test.cpp)
target_link_libraries(tcpb_test PUBLIC libtcpb PRIVATE "${PROTOBUF_LIB}" Threads::Threads)
target_include_directories(tcpb_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
install(TARGETS tcpb_test DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)
