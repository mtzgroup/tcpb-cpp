# executables

add_executable(test_api_fortran fortran/test_api.F90)
target_link_libraries(test_api_fortran PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_fortran DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/fortran)
install(FILES ${tcpb_SOURCE_DIR}/examples/api/fortran/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/fortran/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/fortran)

add_executable(test_api_fortran_openmm fortran_openmm/test_api.F90)
target_link_libraries(test_api_fortran_openmm PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_fortran_openmm DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/fortran_openmm)
install(FILES ${tcpb_SOURCE_DIR}/examples/api/fortran_openmm/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/fortran_openmm/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/fortran_openmm)

add_executable(test_api_cpp cpp/test_api.cpp)
target_link_libraries(test_api_cpp PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_cpp DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/cpp)
install(FILES ${tcpb_SOURCE_DIR}/examples/api/cpp/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/cpp/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/cpp)

add_executable(test_api_cpp_openmm cpp_openmm/test_api.cpp)
target_link_libraries(test_api_cpp_openmm PUBLIC libtcpb PRIVATE protobuf::libprotobuf Threads::Threads)
install(TARGETS test_api_cpp_openmm DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/cpp_openmm)
install(FILES ${tcpb_SOURCE_DIR}/examples/api/cpp_openmm/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/cpp_openmm/reference.log
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/cpp_openmm)

install(FILES ${tcpb_SOURCE_DIR}/examples/api/python/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/python/reference.log
              ${tcpb_SOURCE_DIR}/examples/api/python/test_api.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/python)

install(FILES ${tcpb_SOURCE_DIR}/examples/api/python_openmm/terachem.inp
              ${tcpb_SOURCE_DIR}/examples/api/python_openmm/reference.log
              ${tcpb_SOURCE_DIR}/examples/api/python_openmm/test_api.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/api/python_openmm)