cmake_minimum_required(VERSION 3.16)

project(simplehttp VERSION 1.0.0 DESCRIPTION "A simple C++ HTTP Client and Server with OpenTracing support.")

include(GNUInstallDirs)
find_package(Boost 1.70 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

set(LibSimpleHttpSources src/simplehttpclient.cpp src/simplehttpserver.cpp)
set(LibSimpleHttpPublicHeaders src/simplehttpclient.h src/simplehttpserver.h src/common.h)

add_library(simplehttp SHARED ${LibSimpleHttpSources})
set_target_properties(simplehttp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${LibSimpleHttpPublicHeaders}")

add_library(simplehttp_static STATIC ${LibSimpleHttpSources})
set_target_properties(simplehttp_static PROPERTIES
    OUTPUT_NAME simplehttp
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${LibSimpleHttpPublicHeaders}")

configure_file(simplehttp.pc.in simplehttp.pc @ONLY)

add_executable(testclient src/testclient.cpp)
target_link_libraries(testclient simplehttp opentracing opentracing_mocktracer)

add_executable(testserver src/testserver.cpp)
target_link_libraries(testserver simplehttp opentracing opentracing_mocktracer)

install(TARGETS simplehttp simplehttp_static
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simplehttp)

install(FILES ${CMAKE_BINARY_DIR}/simplehttp.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
