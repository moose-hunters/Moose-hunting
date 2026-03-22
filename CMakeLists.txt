cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(PROJECT_NAME MooseHunting)
project(${PROJECT_NAME} LANGUAGES CXX)

# ========== ПОИСК OPENGL ==========
find_package(OpenGL REQUIRED)

# ========== НАСТРОЙКА GLFW ==========
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

# ========== GLAD ==========
add_subdirectory(external/glad)

# ========== GLM (header-only) ==========
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)

# ========== STB (header-only, создаем .cpp для реализации) ==========
# Создаем stb_image.cpp в папке сборки
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb_image.cpp 
    "#define STB_IMAGE_IMPLEMENTATION\n"
    "#include \"stb_image.h\"\n"
)

add_library(stb_image STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb_image.cpp)
target_include_directories(stb_image PUBLIC external/stb)

# ========== TINYGLTF (header-only) ==========
add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE 
    external/tinygltf
    external/stb                              # tinygltf использует stb_image
)

# ========== ОСНОВНАЯ ПРОГРАММА ==========
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/Window.cpp 
    src/Game.cpp
    src/gl_utils.cpp
    src/Tree.cpp
    src/shader.cpp
)

# ========== ПУТИ ДЛЯ ПОИСКА ЗАГОЛОВКОВ ==========
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

# ========== ЛИНКОВКА ==========
target_link_libraries(${PROJECT_NAME} 
    glfw
    glad
    OpenGL::GL
    glm
    tinygltf
    stb_image
)