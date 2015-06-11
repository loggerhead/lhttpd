cmake_minimum_required (VERSION 2.6)

set (APP_VERSION 0.1)
set (NAME lhttpd)
project (${NAME})
include (CheckLibraryExists)

set (DEBUG 1)
set (APP_NAME ${NAME})
set (DEFAULT_PORT 9999)
set (CMAKE_MACOSX_RPATH 0)

configure_file(config.h.in config.h)

if (${DEBUG})
    set (CMAKE_C_FLAGS "-g -std=gnu99 -Wall -Wno-unused-function ${CMAKE_C_FLAGS}")
else ()
    set (CMAKE_C_FLAGS "-O2 -std=gnu99 -Wall ${CMAKE_C_FLAGS}")
endif()

set (SOURCES ${SOURCES}
        src/http_parser.c
        src/httpclient.c
        src/httpserver.c
        src/httputil.c
        src/util.c)

set (HEADERS ${HEADERS}
        include/${NAME}.h
        include/http_parser.h
        include/uthash.h)

find_library(LIBUV uv)

if (${LIBUV} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libuv")
endif()

include_directories(${PROJECT_BINARY_DIR} "include" "src")

add_library(${NAME} SHARED ${SOURCES})
set_target_properties(${NAME} PROPERTIES
    VERSION ${APP_VERSION}
    SOVERSION 1)
target_link_libraries(${NAME} uv)

install(TARGETS ${NAME}
        LIBRARY DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)