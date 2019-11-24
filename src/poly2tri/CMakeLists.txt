#…Ë÷√ ‰≥ˆ¬∑æ∂
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  ${CMAKE_CURRENT_BINARY_DIR}/../release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${CMAKE_CURRENT_BINARY_DIR}/../debug) 

add_library(poly2tri STATIC
    poly2tri.h
    common/utils.h
    common/shapes.h
    common/shapes.cc
    sweep/cdt.h
    sweep/cdt.cc
    sweep/sweep.h
    sweep/sweep.cc
    sweep/sweep_context.h
    sweep/sweep_context.cc
    sweep/advancing_front.h
    sweep/advancing_front.cc
)
