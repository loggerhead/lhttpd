cmake_minimum_required (VERSION 2.6)

set (NAME lhttpd)
project (${NAME}_test)

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
    set_target_properties(${EXEC} PROPERTIES C_STANDARD 99)
    target_link_libraries(${EXEC} ${NAME} uv)
endforeach()