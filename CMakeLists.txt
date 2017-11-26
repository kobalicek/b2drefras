cmake_minimum_required(VERSION 2.8.12)
project(b2drefras CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SRCS
  base.h
  base.cpp
  performance.h
  performance.cpp
  rasterizer-a1.cpp
  rasterizer-a1.h
  rasterizer-a2.cpp
  rasterizer-a2.h
)

add_executable(render_bench render_bench.cpp ${SRCS})
add_executable(render_cmd   render_cmd.cpp   ${SRCS})
