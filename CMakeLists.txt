cmake_minimum_required(VERSION 3.10)

project(simplehttp VERSION 1.0.1 DESCRIPTION "A simple C++ HTTP Client and Server with OpenTracing support.")

include(GNUInstallDirs)
find_package(Boost 1.70 REQUIRED)
find_package(OpenTracing REQUIRED)

set(CMAKE_CXX_STANDARD 17)

set(LibSimpleHttpSources src/simplehttpclient.cpp src/simplehttpserver.cpp)
set(LibSimpleHttpPublicHeaders src/simplehttpclient.h src/simplehttpserver.h src/common.h)

find_package(Threads)

add_library(simplehttp SHARED ${LibSimpleHttpSources})
set_target_properties(simplehttp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${LibSimpleHttpPublicHeaders}")
target_link_libraries(simplehttp OpenTracing::opentracing ${CMAKE_THREAD_LIBS_INIT})

add_library(simplehttp_static STATIC ${LibSimpleHttpSources})
set_target_properties(simplehttp_static PROPERTIES
    OUTPUT_NAME simplehttp
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON
    PUBLIC_HEADER "${LibSimpleHttpPublicHeaders}")

configure_file(simplehttp.pc.in simplehttp.pc @ONLY)

add_executable(testclient src/testclient.cpp)
target_link_libraries(testclient simplehttp OpenTracing::opentracing opentracing_mocktracer)

add_executable(testserver src/testserver.cpp)
target_link_libraries(testserver simplehttp OpenTracing::opentracing opentracing_mocktracer)

install(TARGETS simplehttp simplehttp_static
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simplehttp)

install(FILES ${CMAKE_BINARY_DIR}/simplehttp.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
