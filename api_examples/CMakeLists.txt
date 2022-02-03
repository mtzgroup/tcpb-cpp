# executables

add_executable(test_api fortran/test_api.F90)
target_link_libraries(test_api PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api DESTINATION ${CMAKE_INSTALL_PREFIX}/api_examples/fortran)
install(FILES ${tcpb_SOURCE_DIR}/api_examples/fortran/terachem.inp
              ${tcpb_SOURCE_DIR}/api_examples/fortran/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/api_examples/fortran)

#add_executable(qmmm-tcpb-example qmmm/tcpb-example.cpp)
#target_link_libraries(qmmm-tcpb-example PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
#install(TARGETS qmmm-tcpb-example DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/qmmm)
#install(FILES ${tcpb_SOURCE_DIR}/examples/qmmm/tc.template DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/qmmm)