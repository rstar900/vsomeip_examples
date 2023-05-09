cmake_minimum_required (VERSION 2.8)

set (CMAKE_CXX_FLAGS "-g -std=c++0x")

set (CMAKE_PREFIX_PATH "/home/rgarg/viss_someip_service/vsomeip_test/vsomeip")

find_package (vsomeip REQUIRED )
find_package( Boost 1.55 COMPONENTS system thread log REQUIRED )

include_directories (
    ${Boost_INCLUDE_DIR}
    ${VSOMEIP_INCLUDE_DIRS}
)

add_executable(service-example src/service-example.cpp)
add_executable(client-example src/client-example.cpp)
target_link_libraries(service-example vsomeip ${Boost_LIBRARIES})
target_link_libraries(client-example vsomeip ${Boost_LIBRARIES})
