cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(PROJECT_NAME MooseHunting)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# ========== ASSIMP ==========
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)

# ========== GLM (header-only) ==========
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)

# ========== STB ==========
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb_image.cpp
    "#define STB_IMAGE_IMPLEMENTATION\n"
    "#include \"stb_image.h\"\n"
)
add_library(stb_image STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb_image.cpp)
target_include_directories(stb_image PUBLIC external/stb)

# ========== ОСНОВНАЯ ПРОГРАММА ==========
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Window.cpp
    src/Game.cpp
    src/Tree.cpp
    src/shader.cpp
)

# ========== ЗАГОЛОВКИ ==========
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp/include
    ${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include
)

# ========== ЛИНКОВКА ==========
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad
    assimp
    glm
    stb_image
    ${OPENGL_LIBRARIES}
)

# ========== КОПИРОВАНИЕ РЕСУРСОВ ==========
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMENT "Copying assets and shaders..."
)