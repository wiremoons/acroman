# cmake build file for amt
#  created 07 May 2017
#  To use run:  mkdir build && cd build && cmake .. && make  
cmake_minimum_required(VERSION 3.6)
project(amt)
#
# enable verbose output
#set(CMAKE_VERBOSE_MAKEFILE on)
#
# cmake -DCMAKE_BUILD_TYPE=Debug
# check if CMake 'debug' build is being used?
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1)
    add_definitions(-DNDEBUG)
    message("CMake build 'DEBUG'")
endif()
#
# add list of c source code files to var ${SOURCES}
file(GLOB SOURCES "./src/*.c")
#
# add location for built binary file
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#
# add additional flags to standard ${CMAKE_C_FLAGS}
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
endif()
#
# show build flags being used:
message("CMake build flags for C: ${CMAKE_C_FLAGS} ${SOURCES} ${CMAKE_DL_LIBS}")
#
# give final executable name and the C source code files required to build it
add_executable(amt ${SOURCES})
#
# setup needed 'pthreads' library to link to as: Threads::Threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
#
# list the names of the C libraries to link against: pthreads
target_link_libraries(amt Threads::Threads ${CMAKE_DL_LIBS})
