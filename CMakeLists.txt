cmake_minimum_required(VERSION 3.2)
project(ecs_bench_over C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

FIND_PACKAGE (Threads)

# Find Eigen 3 library:  https://github.com/RainerKuemmerle/g2o/blob/master/cmake_modules/FindEigen3.cmake
find_package (Eigen3 REQUIRED)

include_directories (${EIGEN3_INCLUDE_DIRS})

add_executable(ecs_bench_over main.cpp)

TARGET_LINK_LIBRARIES(ecs_bench_over pthread)

install(TARGETS ecs_bench_over RUNTIME DESTINATION bin)
