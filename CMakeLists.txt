cmake_minimum_required(VERSION 3.22)
project(cuda_terrain_renderer LANGUAGES CXX CUDA)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../cuda_kernels/include
)

# Source files
set(SOURCES
    src/main.cpp
    src/rgb.cpp
)

# Create executable
add_executable(terrain_renderer ${SOURCES})

# Link prebuilt CUDA kernel DLL
# Assumes lib/cuda_kernels.lib is the import library
target_link_libraries(terrain_renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cuda_kernels.lib
    cudart
)

# Ensure the runtime DLL is available
# (Optional: copy DLL to build output directory)
add_custom_command(TARGET terrain_renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/cuda_kernels.dll
        $<TARGET_FILE_DIR:terrain_renderer>
)

