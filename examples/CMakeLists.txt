# executables

add_executable(qm-tcpb-example qm/tcpb-example.cpp)
target_link_libraries(qm-tcpb-example PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS qm-tcpb-example DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/qm)
install(FILES ${tcpb_SOURCE_DIR}/examples/qm/c2h4.xyz ${tcpb_SOURCE_DIR}/examples/qm/tc.template DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/qm)

add_executable(qmmm-tcpb-example qmmm/tcpb-example.cpp)
target_link_libraries(qmmm-tcpb-example PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS qmmm-tcpb-example DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/qmmm)
install(FILES ${tcpb_SOURCE_DIR}/examples/qmmm/tc.template DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/qmmm)