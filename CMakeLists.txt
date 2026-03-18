cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(PROJECT_NAME MooseHunting)
project(${PROJECT_NAME} LANGUAGES CXX)

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/Window.cpp 
    src/Game.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glad)

target_link_libraries(${PROJECT_NAME} 
    glfw
    glad
)