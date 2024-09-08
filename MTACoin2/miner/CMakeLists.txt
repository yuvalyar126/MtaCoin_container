cmake_minimum_required(VERSION 3.22.1)
project(miner C)

set(CMAKE_C_STANDARD 17)

find_package(ZLIB REQUIRED)

add_executable(miner miner.c ../SharedFile/SharedFile.c)

target_link_libraries(miner z ${ZLIB_LIBRARIES})



