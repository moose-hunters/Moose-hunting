cmake_minimum_required(VERSION 4.2.0 FATAL_ERROR)

project(Glad)

add_library(glad STATIC include/glad/glad.h src/glad.c)
target_include_directories(glad PUBLIC include/)
