cmake_minimum_required(VERSION 3.6)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.126.tar.gz"
    SHA1 "78fb48ee9e636adcadbc61a6895a338733388dc4"
)

project(ecs_bench_over CXX)

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

hunter_add_package(Boost COMPONENTS math)
find_package(Boost CONFIG REQUIRED math_tr1)

hunter_add_package(Eigen)
find_package(Eigen3 CONFIG REQUIRED)

find_package(Threads)

include_directories (${EIGEN3_INCLUDE_DIRS})

add_executable(ecs_bench_over main.cpp)

target_link_libraries(ecs_bench_over PUBLIC pthread Boost::math_tr1 Eigen3::Eigen)

install(TARGETS ecs_bench_over RUNTIME DESTINATION bin)

