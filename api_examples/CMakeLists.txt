# executables

add_executable(test_api_fortran fortran/test_api.F90)
target_link_libraries(test_api_fortran PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_fortran DESTINATION ${CMAKE_INSTALL_PREFIX}/api_examples/fortran)
install(FILES ${tcpb_SOURCE_DIR}/api_examples/fortran/terachem.inp
              ${tcpb_SOURCE_DIR}/api_examples/fortran/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/api_examples/fortran)

add_executable(test_api_cpp cpp/test_api.cpp)
target_link_libraries(test_api_cpp PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_cpp DESTINATION ${CMAKE_INSTALL_PREFIX}/api_examples/cpp)
install(FILES ${tcpb_SOURCE_DIR}/api_examples/cpp/terachem.inp
              ${tcpb_SOURCE_DIR}/api_examples/cpp/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/api_examples/cpp)

install(FILES ${tcpb_SOURCE_DIR}/api_examples/python/terachem.inp
              ${tcpb_SOURCE_DIR}/api_examples/python/reference.log
              ${tcpb_SOURCE_DIR}/api_examples/python/test_api.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/api_examples/python)