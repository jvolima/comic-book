cmake_minimum_required(VERSION 3.16)

project(ComicBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate build/compile_commands.json so editors (VS Code IntelliSense) find include paths such as Qt's.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_UI "Compile the graphical interface using Qt" ON)

enable_testing()

file(GLOB_RECURSE COMIC_CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.hpp
)

add_library(comic_core ${COMIC_CORE_SOURCES})

target_include_directories(comic_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

# libzip reads and writes .cbz (ZIP) archives. PUBLIC because zip_archive.hpp includes zip.h,
# so anything including that header (e.g. the tests) also needs libzip's include path.
find_package(libzip REQUIRED)
target_link_libraries(comic_core PUBLIC libzip::zip)

# ---------- Unit tests (GoogleTest) ----------
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# On Windows, use the same C runtime as the rest of the project.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Only GoogleTest itself is needed: do not build or install GoogleMock.
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB COMIC_CORE_TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/*.cpp
)

add_executable(comic_core_tests ${COMIC_CORE_TEST_SOURCES})

target_link_libraries(comic_core_tests
    PRIVATE
        comic_core
        GTest::gtest_main
)

target_compile_definitions(comic_core_tests PRIVATE
    SAMPLES_DIR="${CMAKE_SOURCE_DIR}/src/samples"
)

include(GoogleTest)
gtest_discover_tests(comic_core_tests)

if(BUILD_UI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    qt_add_executable(comic_reader
        src/ui/main.cpp
        src/ui/comicbookreader.h
        src/ui/comicbookreader.cpp
    )

    target_link_libraries(comic_reader
        PRIVATE
            comic_core
            Qt6::Widgets
    )
endif()
