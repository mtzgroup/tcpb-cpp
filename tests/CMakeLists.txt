# executables

add_executable(input_test input_test.cpp)
target_link_libraries(input_test PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS input_test DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)
install(DIRECTORY ${tcpb_SOURCE_DIR}/tests/input DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)

add_executable(socket_test socket_test.cpp)
target_link_libraries(socket_test PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS socket_test DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)

add_executable(tcpb_test tcpb_test.cpp)
target_link_libraries(tcpb_test PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS tcpb_test DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)
