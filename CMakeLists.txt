cmake_minimum_required(VERSION 2.8)

project(memcomp)

add_compile_options(-std=c++11)

FILE(GLOB SOURCES
    source/*.hpp source/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
