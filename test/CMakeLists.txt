cmake_minimum_required (VERSION 2.6)

set (NAME lhttpd)
project (${NAME}_test)

if (${DEBUG})
    set (CMAKE_C_FLAGS "-g -std=gnu99 -Wall -Wno-unused-function ${CMAKE_C_FLAGS}")
else ()
    set (CMAKE_C_FLAGS "-std=gnu99 -Wall ${CMAKE_C_FLAGS}")
endif()

set (SOURCES ${SOURCES}
        test_httpserver.c
        test_httputil.c
        test_util.c)

find_library(LIBUV uv)
if (${LIBUV} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libuv")
endif()

cmake_policy(SET CMP0015 NEW)
link_directories(..)

foreach (SOURCE ${SOURCES})
    string(REPLACE ".c" "" EXEC ${SOURCE})
    add_executable(${EXEC} ${SOURCE})
    target_link_libraries(${EXEC} ${NAME} uv)
endforeach()