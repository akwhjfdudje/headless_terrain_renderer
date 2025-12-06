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

target_link_libraries(terrainer
    ${CMAKE_CURRENT_SOURCE_DIR}/../cuda_kernels/build/Release/cuda_kernels.lib
    cudart
)

add_custom_command(TARGET terrainer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../cuda_kernels/build/Release/cuda_kernels.dll"
        $<TARGET_FILE_DIR:terrainer>
    COMMENT "Copying cuda_kernels.dll to output directory"
)
