# executables

add_executable(test_api_fortran fortran/test_api.F90)
target_link_libraries(test_api_fortran PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_fortran DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/fortran)
install(FILES ${tcpb_SOURCE_DIR}/examples/api/fortran/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/fortran/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/fortran)

add_executable(test_api_cpp cpp/test_api.cpp)
target_link_libraries(test_api_cpp PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_cpp DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/cpp)
install(FILES ${tcpb_SOURCE_DIR}/examples/api/cpp/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/cpp/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/cpp)

install(FILES ${tcpb_SOURCE_DIR}/examples/api/python/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/python/reference.log
              ${tcpb_SOURCE_DIR}/examples/api/python/test_api.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/python)