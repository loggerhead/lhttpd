cmake_minimum_required (VERSION 2.6)

set (APP_VERSION 0.1)
set (NAME lhttpd)
project (${NAME})
include (CheckLibraryExists)

# Config variable
set (DEBUG 1)
set (APP_NAME ${NAME})
set (CMAKE_MACOSX_RPATH 0)

# Sources and headers
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "include/*.h")

# Libraries
set (LINK_LIBS uv)

find_library(HAS_LIBUV uv)
if (${HAS_LIBUV} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libuv")
endif()

# Optional libraries
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
    set (SOURCES ${SOURCES} src/sqlite.c)
endif()
if (${HAS_HIREDIS})
    set (LINK_LIBS ${LINK_LIBS} hiredis)
    set (SOURCES ${SOURCES} src/redis.c)
endif()

# Compile
if (${DEBUG})
    set (CMAKE_C_FLAGS "-g -std=gnu99 -Wall ${CMAKE_C_FLAGS}")
else ()
    set (CMAKE_C_FLAGS "-O2 -std=gnu99 -Wall -Wno-unused-function ${CMAKE_C_FLAGS}")
endif()

configure_file(config.h.in include/config.h)
include_directories(${PROJECT_BINARY_DIR} "include" "src")
add_library(${NAME} SHARED ${SOURCES})
set_target_properties(${NAME} PROPERTIES
    VERSION ${APP_VERSION}
    SOVERSION 1)
target_link_libraries(${NAME} ${LINK_LIBS})

# Install
set (API_HEADER include/${NAME}.in.h)
LIST(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${API_HEADER})

file(WRITE ${NAME}.h "")
foreach (HEADER ${HEADERS})
    file(READ ${HEADER} CONTENTS)
    file(APPEND ${NAME}.h "${CONTENTS}\n")
endforeach ()
file(READ ${API_HEADER} CONTENTS)
file(APPEND ${NAME}.h "${CONTENTS}\n")

install(TARGETS ${NAME} LIBRARY DESTINATION lib)
install(FILES ${NAME}.h DESTINATION include)