# cmake最低版本号要求
cmake_minimum_required(VERSION 3.16)

# 项目名
project(WebServer)

add_compile_options(-std=c++11 -g -ggdb -Wall -Wextra)

include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/log SRC_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/util SRC_FILE)

add_executable(WebServer main.cpp ${SRC_FILE})
set_target_properties(WebServer PROPERTIES COMPILE_FLAGS "-pthread" LINK_FLAGS "-pthread")
