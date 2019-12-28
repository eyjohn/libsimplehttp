cmake_minimum_required(VERSION 3.16)

project(libsimplehttp_project)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.70 REQUIRED)

set(LibSources src/simplehttpclient.cpp src/simplehttpserver.cpp)
set(PublicHeaders src/simplehttpclient.h src/simplehttpserver.h src/common.h)

add_library(simplehttp SHARED ${LibSources})
add_library(simplehttp_static STATIC ${LibSources})
set_target_properties(simplehttp_static PROPERTIES OUTPUT_NAME simplehttp)

add_executable(testclient src/testclient.cpp)
target_link_libraries(testclient simplehttp opentracing opentracing_mocktracer)

add_executable(testserver src/testserver.cpp)
target_link_libraries(testserver simplehttp opentracing opentracing_mocktracer)

install(TARGETS simplehttp simplehttp_static DESTINATION lib)
install(FILES ${PublicHeaders} DESTINATION include)
