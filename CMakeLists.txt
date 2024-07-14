cmake_minimum_required(VERSION 3.10)  
project(WebServer)  
  
find_package(CURL REQUIRED)
set(CMAKE_CXX_STANDARD 11)  
  
# 添加 include 目录  
include_directories(include)  
include_directories(${CURL_INCLUDE_DIR})
  
# 查找 src 目录下的所有源文件  
file(GLOB SOURCES "src/*.cpp")  
  
# 添加可执行文件  
add_executable(WebServer ${SOURCES} lib/inih/ini.c lib/inih/INIReader.cpp)  
  
# 添加 lib 目录和链接 pthread 库  
target_link_libraries(WebServer pthread)  
target_link_libraries(WebServer ${CURL_LIBRARIES})
  
# # 添加测试  
# enable_testing()  
# add_subdirectory(tests)
