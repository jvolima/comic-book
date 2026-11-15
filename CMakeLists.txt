cmake_minimum_required(VERSION 3.16)

project(ComicBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_UI "Compile the graphical interface using Qt" ON)

add_library(comic_core
    src/core/main.cpp
)

target_include_directories(comic_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

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