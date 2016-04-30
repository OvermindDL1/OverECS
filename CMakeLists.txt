cmake_minimum_required(VERSION 2.6)
project(ecs_bench_over)

add_executable(ecs_bench_over main.cpp)

install(TARGETS ecs_bench_over RUNTIME DESTINATION bin)
