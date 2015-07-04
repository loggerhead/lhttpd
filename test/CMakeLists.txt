cmake_minimum_required (VERSION 2.6)
set (CMAKE_MODULE_PATH "../cmake")
include(HasLibrary)
include(info)

project (${NAME}_test)

set (CMAKE_C_FLAGS "-g -std=gnu99 -Wall ${CMAKE_C_FLAGS}")

set (SOURCES test_httpserver.c
             test_httputil.c
             test_util.c
             test_sqlite.c
             test_redis.c
             test_json.c
             test_webrouter.c)

# Libraries
set (LINK_LIBS uv)

find_library(HAS_LIBUV uv REQUIRED)
if (${LIBUV} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libuv")
endif()

HAS_LIBRARY(json-c JSON_C)
HAS_LIBRARY(sqlite3)
HAS_LIBRARY(hiredis)

if (${HAS_JSON_C})
    set (LINK_LIBS ${LINK_LIBS} json-c)
    message ("Found json-c, test `src/json.c`")
endif()
if (${HAS_SQLITE3})
    set (LINK_LIBS ${LINK_LIBS} sqlite3)
    message ("Found sqlite3, test `src/sqlite.c`")
endif()
if (${HAS_HIREDIS})
    set (LINK_LIBS ${LINK_LIBS} hiredis)
    message ("Found hiredis, test `src/redis.c`")
endif()

# Compile and link
cmake_policy(SET CMP0015 NEW)
link_directories(..)

foreach (SOURCE ${SOURCES})
    string(REPLACE ".c" "" EXEC ${SOURCE})
    add_executable(${EXEC} ${SOURCE})
    target_link_libraries(${EXEC} ${NAME} ${LINK_LIBS})
endforeach()