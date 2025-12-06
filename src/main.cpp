#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

// Declare your CUDA function
extern "C" void generateHeightmap(
    float* heightmap, int width, int height,
    float scale, int seed, float mix_ratio
);

// Convert [-1,1] to [0,255]
static inline unsigned char toByte(float h) {
    h = std::fmax(-1.0f, std::fmin(1.0f, h));
    return (unsigned char)((h + 1.0f) * 0.5f * 255.0f);
}

int main() {
    const int width  = 1024;
    const int height = 1024;

    // Noise settings
    float scale      = 80.0f;   // larger = smoother
    int seed         = 1337;
    float mix_ratio  = 0.35f;   // 0=Perlin, 1=Voronoi

    std::vector<float> heightmap(width * height);

    generateHeightmap(heightmap.data(), width, height, scale, seed, mix_ratio);

    std::ofstream out("terrain.ppm", std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";

    for (int i = 0; i < width * height; i++) {
        unsigned char g = toByte(heightmap[i]);
        out << g << g << g; // grayscale RGB
    }

    out.close();

    std::cout << "terrain.ppm written successfully.\n";
    return 0;
}

