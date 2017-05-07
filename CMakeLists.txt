# cmake build file for amt
#  created 07 May 2017
#  To use run:  cmake . && make  
cmake_minimum_required(VERSION 3.6)
project(amt)
#
# enable verbose output
#set(CMAKE_VERBOSE_MAKEFILE on)
#
# add list of c source code files to var ${SOURCES}
file(GLOB SOURCES "./*.c")
#
# add additional flags to standard ${CMAKE_C_FLAGS}
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall")
#
# give final executable name and the C source code files required to build it
add_executable(amt ${SOURCES})
#
# setup needed 'pthreads' library to link to as: Threads::Threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
#
#
# GNU Readline library finder
if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY AND NCURSES_LIBRARY)
  set(READLINE_FOUND TRUE)
else(READLINE_INCLUDE_DIR AND READLINE_LIBRARY AND NCURSES_LIBRARY)
  FIND_PATH(READLINE_INCLUDE_DIR readline/readline.h /usr/include/readline)
  FIND_LIBRARY(READLINE_LIBRARY NAMES readline)
        include(FindPackageHandleStandardArgs)
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(Readline DEFAULT_MSG READLINE_INCLUDE_DIR READLINE_LIBRARY )
  MARK_AS_ADVANCED(READLINE_INCLUDE_DIR READLINE_LIBRARY)
endif(READLINE_INCLUDE_DIR AND READLINE_LIBRARY AND NCURSES_LIBRARY)
#
# list the names of the C libraries to link against: pthreads readline dl
target_link_libraries(amt Threads::Threads readline ${CMAKE_DL_LIBS})
