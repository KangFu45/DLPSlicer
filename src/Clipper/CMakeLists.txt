#…Ë÷√ ‰≥ˆ¬∑æ∂
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  ${CMAKE_CURRENT_BINARY_DIR}/../release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${CMAKE_CURRENT_BINARY_DIR}/../debug) 

add_library(clipper STATIC
    clipper.cpp
    clipper.hpp
)
