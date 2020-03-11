#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h")
FILE(GLOB Slic3rLib_SRC_FILES "libslic3r/*.cpp")
FILE(GLOB Slic3rLib_HEAD_FILES "libslic3r/*.h" "libslic3r/*.hpp")

#设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  ${CMAKE_CURRENT_BINARY_DIR}/../release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${CMAKE_CURRENT_BINARY_DIR}/../debug) 

#设置qt cmake路径
if(NOT DEFINED CMAKE_PREFIX_PATH)
	set(CMAKE_PREFIX_PATH C:/Qt/Qt5.12.0/5.12.0/msvc2015_64/lib/cmake)
endif()

# 开启Qt代码自动生成，不再需用自己手写了。按顺序，分别是`Q_OBJECT`宏展开，资源文件，界面文件。
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 使用Qt的模块，写法和.pro文件类似
set(QT Core Gui Widgets OpenGL)
find_package(Qt5 COMPONENTS ${QT} REQUIRED)

# 添加ui与qrc文件 ,qrc文件放在资源文件下
#qt5_wrap_ui(ui_FILES mainwindow.ui)
set(QRC_SOURCE_DIR ${PROJECT_SOURCE_DIR}/resources/qrc)
qt5_add_resources(QRC_FILES ${QRC_SOURCE_DIR}/Resource.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libslic3r)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Clipper)
#link_directories(DLPSlicer ${PROJECT_SOURCE_DIR}/lib-64/lib/quazip)
link_directories(DLPSlicer ${PROJECT_SOURCE_DIR}/lib-64/lib/glut)
add_definitions("/bigobj")

add_executable(DLPSlicer ${Slic3rLib_SRC_FILES} ${Slic3rLib_HEAD_FILES} ${QRC_FILES} ${SRC_FILES} ${HEAD_FILES})

target_link_libraries(DLPSlicer admesh clipper poly2tri polypartition)
target_link_libraries(DLPSlicer glu32.lib opengl32.lib)
#链接第三方库
#target_link_libraries(DLPSlicer optimized quazip.lib)
#target_link_libraries(DLPSlicer debug quazipd.lib)

# 在CMake中使用Qt最快捷的方式，一句代码搞定
qt5_use_modules(DLPSlicer ${QT})