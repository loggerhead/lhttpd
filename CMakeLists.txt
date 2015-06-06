cmake_minimum_required (VERSION 2.6)

set (APP_VERSION 0.1)

set (NAME lhttpd)
project (${NAME})

include (CheckLibraryExists)

set (ON 1)
set (OFF 0)

set (APP_NAME ${NAME})
set (DEBUG ${ON})
set (DEFAULT_PORT 9999)

configure_file(config.h.in config.h)

if (${DEBUG})
    set (CMAKE_C_FLAGS "-g -std=gnu99 -Wall ${CMAKE_C_FLAGS}")
else ()
    set (CMAKE_C_FLAGS "-std=gnu99 -Wall ${CMAKE_C_FLAGS}")
endif()

set (SOURCES ${SOURCES}
        src/httpclient.c
        src/httpserver.c
        src/httputil.c
        src/util.c)

find_library(LIBUV uv)
find_library(LIBJSON json-c)

if (${LIBUV} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libuv")
elseif (${LIBJSON} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libjson-c")
endif()

include_directories(${PROJECT_BINARY_DIR} "include" "src")

add_library (${NAME} ${SOURCES})
target_link_libraries (${NAME} uv json-c)

install (TARGETS ${NAME} DESTINATION lib)
install (FILES ${NAME}.h DESTINATION include)
