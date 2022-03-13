# cmake最低版本号要求
cmake_minimum_required(VERSION 3.16)

# 项目名
project(WebServer)

add_compile_options(-std=c++11 -g -ggdb -Wall -Wextra)

include_directories(${PROJECT_SOURCE_DIR}/include/third_lib)
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${PROJECT_SOURCE_DIR}/lib)

# 检测yaml-cpp静态库/动态库是否存在。
# 如果存在，yamlcpp_lib变量记录的就是libyaml-cpp.a的路径。如果不存在，yamlcpp_lib变量就是"yamlcpp_lib-NOTFOUND"
# 注意这个yamlcpp_lib变量会缓存下来，所以每次cmake之前，如果没有清除缓存文件，yamlcpp_lib就是上一次的值
# NO_CACHE指定yamlcpp_lib不作为缓存变量，作为普通变量
find_library(yamlcpp_lib yaml-cpp ${PROJECT_SOURCE_DIR}/lib NO_CACHE)
# exists后面是一个文件或者目录路径，只有合法存在，就返回ture
IF(EXISTS ${yamlcpp_lib})
    message(STATUS "libyaml-cpp.a located in ${yamlcpp_lib}")
ELSE()
    message(FATAL_ERROR "libyaml-cpp.a not found!")
ENDIF()

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/log SRC_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/util SRC_FILE)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(WebServer main.cpp ${SRC_FILE})
set_target_properties(WebServer PROPERTIES COMPILE_FLAGS "-pthread" LINK_FLAGS "-pthread")
target_link_libraries(WebServer yaml-cpp)
