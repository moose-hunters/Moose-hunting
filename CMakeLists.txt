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
    src/Terrain.cpp)

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

# Говорим CMake собрать ENet из папки external
add_subdirectory(external/enet)

# ---------------------------------------------------
# НАСТРОЙКА КЛИЕНТА (Твоя игра)
# ---------------------------------------------------
# (Предполагаю, что у тебя уже есть add_executable(MooseHunting ...))

# Добавляем путь к заголовочным файлам ENet для игры
target_include_directories(MooseHunting PRIVATE external/enet/include)

# Линкуем ENet к игре (вместе с твоими glfw, glad и т.д.)
target_link_libraries(MooseHunting PRIVATE enet)

if(WIN32)
    target_link_libraries(MooseHunting PRIVATE ws2_32 winmm)
endif()


# ---------------------------------------------------
# НАСТРОЙКА СЕРВЕРА
# ---------------------------------------------------
# Создай файл server.cpp (можешь положить его в src/)
add_executable(GameServer src/server.cpp)

# Добавляем пути к инклудам (чтобы сервер видел Protocol.h из папки include и ENet)
target_include_directories(GameServer PRIVATE include external/enet/include)

target_link_libraries(GameServer PRIVATE enet)

if(WIN32)
    target_link_libraries(GameServer PRIVATE ws2_32 winmm)
endif()