cmake_minimum_required(VERSION 3.18)
project(terrainer LANGUAGES CXX CUDA)

# Enable modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make sure NVCC uses a decent architecture
set(CMAKE_CUDA_ARCHITECTURES 52 60 61 70 75 86 89)

# Source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(terrainer ${SOURCES})

# Link prebuilt CUDA kernel DLL
# Assumes lib/cuda_kernels.lib is the import library
target_link_libraries(terrainer
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cuda_kernels.lib
    cudart
)
