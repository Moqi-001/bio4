cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()

project(VitaSQLite3)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set_source_files_properties(
        sqlite3.c
        PROPERTIES
        COMPILE_FLAGS "-fno-fast-math"
)

add_definitions(-DSQLITE_OS_OTHER=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_THREADSAFE=0)

add_library(VitaSQLite3 STATIC sqlite3.c vita_sqlite.c)

set_target_properties(VitaSQLite3 PROPERTIES
        OUTPUT_NAME "vitaSQLite3"
        PREFIX ""
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
)
