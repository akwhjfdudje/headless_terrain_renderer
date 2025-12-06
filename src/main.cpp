#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

// Declare your CUDA function
extern "C" void generateHeightmap(
    float* heightmap, int width, int height,
    float scale, int seed, float mix_ratio
);

struct RGB { unsigned char r, g, b; };

RGB heightToColor(float h) {
    // h is in [-1, 1], remap to [0, 1]
    float t = (h + 1.0f) * 0.5f;

    // Terrain bands
    if (t < 0.3f) {
        // Water
        return { 25, 129, 226 };
    }
    else if (t < 0.45f) {
        // Grass
        float f = (t - 0.3f) / 0.15f;
        return { 
            (unsigned char)(20 + f * 40),
            (unsigned char)(80 + f * 100),
            (unsigned char)(20 + f * 40)
        };
    }
    else if (t < 0.75f) {
        // Light grass / moss
        float f = (t - 0.45f) / 0.30f;
        return {
            (unsigned char)(60 + f * 80),
            (unsigned char)(140 + f * 90),
            (unsigned char)(40 + f * 40)
        };
    }
    else {
        // Snow
        float f = (t - 0.75f) / 0.25f;
        return {
            (unsigned char)(180 + f * 75),
            (unsigned char)(180 + f * 75),
            (unsigned char)(180 + f * 75)
        };
    }
}

void writeColorPPM(const std::string& filename, const float* hmap, int width, int height) {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";

    for (int i = 0; i < width * height; i++) {
        RGB c = heightToColor(hmap[i]);
        out.write((char*)&c, 3);
    }
    out.close();

    std::cout << "Wrote " << filename << "\n";
}

// Convert [-1,1] to [0,255]
static inline unsigned char toByte(float h) {
    h = std::fmax(-1.0f, std::fmin(1.0f, h));
    return (unsigned char)((h + 1.0f) * 0.5f * 255.0f);
}

int main() {
    const int width  = 1024;
    const int height = 1024;

    // Noise settings
    float scale      = 25.0f;   // larger = smoother
    int seed         = 69420;
    float mix_ratio  = 0.55f;   // 0=Perlin, 1=Voronoi

    std::vector<float> heightmap(width * height);

    generateHeightmap(heightmap.data(), width, height, scale, seed, mix_ratio);

    writeColorPPM("terrain.ppm", heightmap.data(), width, height);
    return 0;
}

