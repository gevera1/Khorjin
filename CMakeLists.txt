cmake_minimum_required(VERSION 2.8)

project(libieee_C37_118)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_SUPPRESS_REGENERATION true)

# feature checks
include(CheckLibraryExists)
check_library_exists(rt clock_gettime "time.h" CONFIG_SYSTEM_HAS_CLOCK_GETTIME)

# check if we are on a little or a big endian
include (TestBigEndian)
test_big_endian(PLATFORM_IS_BIGENDIAN)

option(BUILD_EXAMPLES "Build the examples" ON)

# advanced options
option(DEBUG "Enable debugging mode (include assertions)" OFF)
option(DEBUG_SOCKET "Enable printf debugging for socket layer" OFF)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/config
    src/common/inc
    src/pal/inc
    src/ieeeC37_118
)

set(API_HEADERS 
    src/pal/inc/pal_time.h 
    src/pal/inc/pal_thread.h
	src/ieeeC37_118/ieeeC37_118.h
    src/common/inc/libieeeC37_118_common_api.h
)

IF(WIN32)
include_directories(
    src/vs
)
ENDIF(WIN32)

# write the detected stuff to this file
configure_file(config/stack_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config/stack_config.h)

if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif(BUILD_EXAMPLES)

add_subdirectory(src)

INSTALL(FILES ${API_HEADERS} DESTINATION include/libieeeC37_118)

