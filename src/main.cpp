#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

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

RGB shadeTerrain(const std::vector<float>& hmap, int x, int y, int width, int height) {
    int xm = std::max(x - 1, 0);
    int xp = std::min(x + 1, width - 1);
    int ym = std::max(y - 1, 0);
    int yp = std::min(y + 1, height - 1);

    float dzdx = (hmap[y * width + xp] - hmap[y * width + xm]) * 0.5f;
    float dzdy = (hmap[yp * width + x] - hmap[ym * width + x]) * 0.5f;

    // Light direction
    float lx = 0.5f, ly = 0.5f, lz = 1.0f;
    float len = std::sqrt(lx*lx + ly*ly + lz*lz);
    lx /= len; ly /= len; lz /= len;

    // Normal vector
    float nx = -dzdx, ny = -dzdy, nz = 1.0f;
    len = std::sqrt(nx*nx + ny*ny + nz*nz);
    nx /= len; ny /= len; nz /= len;

    float diff = std::max(0.0f, nx*lx + ny*ly + nz*lz); // diffuse shading

    RGB base = heightToColor(hmap[y * width + x]);
    return { (unsigned char)(base.r * diff), (unsigned char)(base.g * diff), (unsigned char)(base.b * diff) };
}

void writeColorPPM(const std::string& filename, const std::vector<float>& hmap, int width, int height) {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGB c = shadeTerrain(hmap, x, y, width, height);
            out.write((char*)&c, 3);
        }
    }
    out.close();
    std::cout << "Wrote " << filename << "\n";
}

void writeSideViewPPM(const std::string& filename, const std::vector<float>& hmap, int width, int height) {
    const int outWidth  = width;
    const int outHeight = height / 2; // vertical scaling
    std::vector<RGB> image(outWidth * outHeight, {135, 206, 235}); // sky

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // Map height [-1,1] to pixel row
            int row = outHeight - 1 - int((hmap[y * width + x] + 1.0f) * 0.5f * outHeight);
            row = std::clamp(row, 0, outHeight - 1);

            // Fill from row to bottom
            for (int r = row; r < outHeight; r++) {
                image[r * outWidth + x] = heightToColor(hmap[y * width + x]);
            }
        }
    }

    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << outWidth << " " << outHeight << "\n255\n";
    out.write(reinterpret_cast<char*>(image.data()), outWidth * outHeight * 3);
    out.close();
    std::cout << "Wrote " << filename << "\n";
}

void writeIsometricPPM(const std::string& filename, const std::vector<float>& hmap, int width, int height) {
    int outWidth  = width;
    int outHeight = height;
    std::vector<RGB> image(outWidth * outHeight, {0, 0, 0});

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int isoX = (x - y) + height / 2;
            int isoY = (x + y) / 2 - int(hmap[y * width + x] * outHeight / 2);
            
            if (isoX >= 0 && isoX < outWidth && isoY >= 0 && isoY < outHeight) {
                image[isoY * outWidth + isoX] = shadeTerrain(hmap, x, y, width, height);
            }
        }
    }

    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << outWidth << " " << outHeight << "\n255\n";
    out.write(reinterpret_cast<char*>(image.data()), outWidth * outHeight * 3);
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
    float scale      = 500.0f;   // larger = smoother
    int seed         = 4132778;
    float mix_ratio  = 0.75f;   // 0=Perlin, 1=Voronoi

    std::vector<float> heightmap(width * height);

    generateHeightmap(heightmap.data(), width, height, scale, seed, mix_ratio);

    writeColorPPM("terrain.ppm", heightmap, width, height);
    writeSideViewPPM("terrain_side.ppm", heightmap, width, height);
    writeIsometricPPM("terrain_iso.ppm", heightmap, width, height);
    return 0;
}

