project(DLPSlicer)
cmake_minimum_required(VERSION 3.8)
# 本次使用C++标准版本
set(CMAKE_CXX_STANDARD 11)

set (DLPSlicer_VERSION_MAJOR 1)
set (DLPSlicer_VERSION_MINOR 6)
# 加入一个配置头文件，用于处理 CMake 对源码的设置
configure_file (
"${PROJECT_SOURCE_DIR}/config.h.in"
"${PROJECT_SOURCE_DIR}/config.h"
)

if(WIN32 AND NOT MINGW)
  if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "_d")
  endif()
  if(NOT DEFINED CMAKE_RELEASE_POSTFIX)
    set(CMAKE_RELEASE_POSTFIX "_r")
  endif()
  if(NOT DEFINED CMAKE_RELWITHDEBINFO_POSTFIX)
    set(CMAKE_RELWITHDEBINFO_POSTFIX "_r")
  endif()
  if(NOT DEFINED CMAKE_MINSIZEREL_POSTFIX)
    set(CMAKE_MINSIZEREL_POSTFIX "_r")
  endif()
endif()

add_subdirectory(src)

## 指定安装路径
#install (TARGETS DLPSlicer DESTINATION bin)
#install (FILES "${PROJECT_SOURCE_DIR}/config.h"
#DESTINATION include)
## 构建一个 CPack 安装包
#include (InstallRequiredSystemLibraries)
#set (CPACK_RESOURCE_FILE_LICENSE
#"${PROJECT_SOURCE_DIR}/License")
#set (CPACK_PACKAGE_VERSION_MAJOR "${DLPSlicer_VERSION_MAJOR}")
#set (CPACK_PACKAGE_VERSION_MINOR "${DLPSlicer_VERSION_MINOR}")
#include (CPack)