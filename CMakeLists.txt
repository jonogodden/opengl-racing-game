cmake_minimum_required(VERSION 3.16)

# Project name and language
project(opengl_racing_game VERSION 1.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# Include directories
# ----------------------------
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# ----------------------------
# Source files
# ----------------------------
set(SOURCES
    src/main.cpp
    src/glad.c
)

# ----------------------------
# Add executable
# ----------------------------
add_executable(${PROJECT_NAME} ${SOURCES})

# ----------------------------
# Link libraries
# ----------------------------
# GLFW static library (your lib folder)
target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/lib/libglfw3dll.a")

# OpenGL library (platform-specific)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY} ${OpenGL_LIBRARY})
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} OpenGL::GL)
endif()

# ----------------------------
# Copy DLLs to output folder (so it runs)
# ----------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/glfw3.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
