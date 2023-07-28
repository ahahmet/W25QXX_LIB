cmake_minimum_required(VERSION 3.1)


project(flash)

set(SRC
    ./src/w25qxx.c
)

add_library(${PROJECT_NAME}  SHARED ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ./inc)

