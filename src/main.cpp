#include <windows.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <vector>
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "fused/fused.cuh"
#include "render/render.cuh"

void convertNormalsToRGB(
    const float3* normals, int W, int H,
    std::vector<unsigned char>& outRGB);

int main() {
    const int W = 512;
    const int H = 512;
    const float scale = 4.0f;
    const float seed = 123;
    const float mix_ratio = 0.3f;

    float* d_heightmap = nullptr;
    cudaMalloc(&d_heightmap, W * H * sizeof(float));

    generateHeightmap(d_heightmap, W, H, scale, seed, mix_ratio);

    std::vector<float3> vertices(W * H);
    std::vector<float3> normals(W * H);
    std::vector<float2> texcoords(W * H);

    float3* d_vertices;
    float3* d_normals;
    float2* d_texcoords;

    cudaMalloc(&d_vertices,  W * H * sizeof(float3));
    cudaMalloc(&d_normals,   W * H * sizeof(float3));
    cudaMalloc(&d_texcoords, W * H * sizeof(float2));

    cudaGraphicsResource_t resVert  = (cudaGraphicsResource_t)d_vertices;
    cudaGraphicsResource_t resNorm  = (cudaGraphicsResource_t)d_normals;
    cudaGraphicsResource_t resTex   = (cudaGraphicsResource_t)d_texcoords;

    // Generate the mesh into the above buffers
    generateMeshToVBOs(
        d_heightmap, W, H, scale,
        resVert, resNorm, resTex
    );

    cudaMemcpy(normals.data(), d_normals, W * H * sizeof(float3), cudaMemcpyDeviceToHost);

    std::vector<unsigned char> rgb;
    convertNormalsToRGB(normals.data(), W, H, rgb);

    stbi_write_png("terrain_normals.png", W, H, 3, rgb.data(), W * 3);
    std::cout << "Wrote terrain_normals.png\n";

    // Cleanup
    cudaFree(d_heightmap);
    cudaFree(d_vertices);
    cudaFree(d_normals);
    cudaFree(d_texcoords);

    return 0;
}

