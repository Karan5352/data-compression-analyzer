cmake_minimum_required(VERSION 3.15)
project(DataCompressionAnalyzer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# Add nlohmann-json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Add tinyfiledialogs
FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://github.com/native-toolkit/tinyfiledialogs.git
    GIT_TAG master
)
FetchContent_MakeAvailable(tinyfiledialogs)

# Set tinyfiledialogs source file language to C
set_source_files_properties(${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c PROPERTIES LANGUAGE C)

# Set source files
set(SOURCES
    src/main.cpp
    src/gui/MainWindow.cpp
    src/compression/Compressor.cpp
    src/compression/GzipCompressor.cpp
    src/compression/ArchiveCompressor.cpp
    src/utils/FileHandler.cpp
)

# Set header files
set(HEADERS
    src/gui/MainWindow.h
    src/compression/Compressor.h
    src/compression/GzipCompressor.h
    src/compression/ArchiveCompressor.h
    src/utils/FileHandler.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add ImGui source files
target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${json_SOURCE_DIR}/include
    ${tinyfiledialogs_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${ZLIB_LIBRARIES}
    OpenGL::GL
    glfw
)

# Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
) 