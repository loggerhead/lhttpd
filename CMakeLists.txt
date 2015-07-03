cmake_minimum_required (VERSION 2.6)
set (CMAKE_MODULE_PATH "cmake")
include (HasLibrary)
include (info)

project (${NAME})
include (CheckLibraryExists)

# Sources and headers
file (GLOB_RECURSE SOURCES "src/*.c")
file (GLOB_RECURSE HEADERS "include/*.h")

# Libraries
set (LINK_LIBS uv)

find_library(HAS_LIBUV uv)
if (${HAS_LIBUV} MATCHES ".*-NOTFOUND")
    message(FATAL_ERROR "Not found libuv")
endif()

# Optional libraries
HAS_LIBRARY(json-c JSON_C)
HAS_LIBRARY(sqlite3)
HAS_LIBRARY(hiredis)

if (${HAS_JSON_C})
    set (LINK_LIBS ${LINK_LIBS} json-c)
else()
    LIST(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/json.c)
endif()
if (${HAS_SQLITE3})
    set (LINK_LIBS ${LINK_LIBS} sqlite3)
else()
    LIST(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite.c)
endif()
if (${HAS_HIREDIS})
    set (LINK_LIBS ${LINK_LIBS} hiredis)
else()
    LIST(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/redis.c)
endif()

# Compile
if (${DEBUG})
    set (CMAKE_C_FLAGS "-g -std=gnu99 -Wall ${CMAKE_C_FLAGS}")
else()
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