cmake_minimum_required(VERSION 3.16)
project(JsonParser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(JsonParser WIN32
    main.cpp
    json_utils.cpp
    fold_region.cpp
    json_syntax_highlighter.cpp
    gutter_widget.cpp
    json_editor_widget.cpp
    search_bar.cpp
    search_results_list.cpp
    status_bar.cpp
    main_window.cpp
    app.rc
    resources.qrc
)

target_link_libraries(JsonParser PRIVATE Qt6::Widgets)

target_include_directories(JsonParser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Windows deployment
if(WIN32)
    add_custom_command(TARGET JsonParser POST_BUILD
        COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe
            $<TARGET_FILE:JsonParser>
            --no-translations
            --no-opengl-sw
    )
endif()