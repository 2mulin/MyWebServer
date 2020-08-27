cmake_minimum_required(VERSION 2.8)

# 项目名
project(WebServer)

# 将给定的路径添加到编译器搜索包含文件（.h文件）的路径列表中。
include_directories(include)

# 这是2.8版本的cmake，用CMAKE_CXX_FLAGS变量设置
set(CMAKE_CXX_FLAGS "-std=c++11")
# 若是>3.1版本,可以使用CMAKE_CXX_STANDARD变量
# set(CMAKE_CXX_STANDARD 11)

# 设置源文件目录为src，并将其赋值给环境变量DIR_SRCS
aux_source_directory(src DIR_SRCS)

# 将变量DIR_SRCS的源文件编译成一个名为WebServer的可执行文件
add_executable(WebServer main.cpp ${DIR_SRCS})