# ���ӵ�����TBB��
set(BOOST170_INC_DIR C:/Github/DLPSlicer/lib-64/include)
set(BOOST170_LINK_DIR C:/Github/DLPSlicer/lib-64/lib/boost)

add_definitions("-D _CRT_SECURE_NO_WARNINGS")
add_definitions("-D _USE_MATH_DEFINES")

add_library(admesh STATIC
    stl.h
    connect.c
    normals.c
    shared.c
    stl_io.c
    stlinit.c
    util.c
)

target_include_directories(admesh PUBLIC ${BOOST170_INC_DIR})
target_link_directories(admesh PUBLIC ${BOOST170_LINK_DIR})
