cmake_minimum_required (VERSION 2.6)

set (NAME lhttpd)
project (${NAME}_test)

set (CMAKE_C_FLAGS "-g -std=gnu99 -Wall ${CMAKE_C_FLAGS}")

# file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set (SOURCES test_httpserver.c
             test_httputil.c
             test_util.c
             test_webrouter.c)

# Libraries
set (LINK_LIBS uv)

find_library(HAS_LIBUV uv REQUIRED)
if (${LIBUV} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libuv")
endif()

MACRO(HAS_LIBRARY lib)
    find_library(IS_EXIST_${lib} ${lib})
    string(TOUPPER ${lib} LIB)
    IF (${IS_EXIST_${lib}} MATCHES ".*-NOTFOUND")
        SET(HAS_${LIB} 0)
    ELSE()
        SET(HAS_${LIB} 1)
    ENDIF()
ENDMACRO(HAS_LIBRARY)

HAS_LIBRARY(sqlite3)
HAS_LIBRARY(hiredis)

if (${HAS_SQLITE3})
    set (LINK_LIBS ${LINK_LIBS} sqlite3)
    set (SOURCES ${SOURCES} test_sqlite.c)
endif()
if (${HAS_HIREDIS})
    set (LINK_LIBS ${LINK_LIBS} hiredis)
    set (SOURCES ${SOURCES} test_redis.c)
endif()

# Compile and link
message(${SOURCES})
cmake_policy(SET CMP0015 NEW)
link_directories(..)

foreach (SOURCE ${SOURCES})
    string(REPLACE ".c" "" EXEC ${SOURCE})
    add_executable(${EXEC} ${SOURCE})
    target_link_libraries(${EXEC} ${NAME} ${LINK_LIBS})
endforeach()