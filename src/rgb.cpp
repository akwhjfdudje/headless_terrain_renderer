#include <windows.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

/**
 * Convert array of float3 normals into 8-bit RGB.
 * n = [-1,1] -> [0,255]
 */
void convertNormalsToRGB(
    const float3* normals,
    int width,
    int height,
    std::vector<unsigned char>& outRGB) {

    outRGB.resize(width * height * 3);

    for (int i = 0; i < width * height; ++i) {
        float nx = normals[i].x;
        float ny = normals[i].y;
        float nz = normals[i].z;

        // map [-1,1] → [0,255]
        outRGB[i*3 + 0] = (unsigned char)( (nx * 0.5f + 0.5f) * 255.0f );
        outRGB[i*3 + 1] = (unsigned char)( (ny * 0.5f + 0.5f) * 255.0f );
        outRGB[i*3 + 2] = (unsigned char)( (nz * 0.5f + 0.5f) * 255.0f );
    }
}
