cmake_minimum_required(VERSION 3.22.1)

# Project name
project(ServerProject)

# Set the C standard
set(CMAKE_C_STANDARD 17)

# Add source files
set(SOURCES server.c ../SharedFile/SharedFile.c)

# Include directories
include_directories(.)

# Include directories for shared files
include_directories(../SharedFile)

# Find and link zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# Add the executable
add_executable(server ${SOURCES})

# Link libraries
target_link_libraries(server ${ZLIB_LIBRARIES} pthread)
