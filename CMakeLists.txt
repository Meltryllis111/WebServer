cmake_minimum_required(VERSION 3.10)
project(WebServer)

set(CMAKE_CXX_STANDARD 11)

# 添加 include 目录
include_directories(include)

# 查找 src 目录下的所有源文件
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(your_project ${SOURCES} lib/inih/ini.c lib/inih/INIReader.cpp)

# 添加测试
enable_testing()
add_subdirectory(tests)
