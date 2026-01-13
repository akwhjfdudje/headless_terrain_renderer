#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

extern "C" void generateHeightmap(
    float* heightmap, int width, int height,
    float scale, int seed, float mix_ratio
);

extern "C" void normalizeHeightmap(float* heightmap, int width, int height, float min_val, float max_val);

extern "C" void erodeHeightmap(
    float* heightmap,
    float* watermap,
    float* sedimentmap,
    int width,
    int height,
    float timeStep,
    float rainAmount,
    float evapRate,
    float capacity,
    float depositRate,
    float erosionRate);

struct RGB { unsigned char r, g, b; };

RGB heightToColor(float h) {
    // h is in [-1, 1], remap to [0, 1]
    float t = (h + 1.0f) * 0.5f;

    if (t < 0.30f) {
        // Deep water → shallow water
        float f = t / 0.30f;
        return {
            (unsigned char)(10 + f * 40),
            (unsigned char)(60 + f * 70),
            (unsigned char)(140 + f * 100)
        };
    }
    else if (t < 0.50f) {
        // Grasslands
        float f = (t - 0.30f) / 0.20f;
        return {
            (unsigned char)(30 + f * 50),
            (unsigned char)(100 + f * 110),
            (unsigned char)(30 + f * 40)
        };
    }
    else if (t < 0.75f) {
        // Rocky slopes
        float f = (t - 0.50f) / 0.25f;
        return {
            (unsigned char)(80 + f * 60),
            (unsigned char)(85 + f * 55),
            (unsigned char)(70 + f * 50)
        };
    }
    else {
        // Snow
        float f = (t - 0.75f) / 0.25f;
        return {
            (unsigned char)(200 + f * 55),
            (unsigned char)(200 + f * 55),
            (unsigned char)(200 + f * 55)
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

void writeIsometricPPM(const std::string& filename, const std::vector<float>& hmap, int width, int height) {
    int outWidth  = width * 2;
    int outHeight = height * 2;
    float zoom = 1.0f;
    std::vector<RGB> image(outWidth * outHeight, {0, 0, 0});

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int isoX = (x - y) * zoom + outWidth / 2;
            int isoY = (x + y) * zoom / 2 - int(hmap[y * width + x] * outHeight / (2 * zoom)) + outHeight / 3;
            
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
    float scale      = 700.0f;   // larger = smoother
    std::srand(std::time({})); // use current time as seed for random generator
    int seed         = std::rand(); //4132778;
    float mix_ratio  = 0.6f;   // 0=Perlin, 1=Voronoi

    std::vector<float> heightmap(width * height);
    std::vector<float> watermap(width * height, 0.0f);
    std::vector<float> sedimentmap(width * height, 0.0f);

    generateHeightmap(heightmap.data(), width, height, scale, seed, mix_ratio);

    // Erode: 
    float timeStep    = 4.0f;
    float rainAmount  = 0.5f;
    float evapRate    = 0.03f;
    float capacity    = 4.0f;
    float depositRate = 0.5f;
    float erosionRate = 0.5f;

    for (int i = 0; i < 7000; i++) 
        erodeHeightmap(heightmap.data(), watermap.data(), sedimentmap.data(),
                   width, height, timeStep, rainAmount, evapRate,
                   capacity, depositRate, erosionRate);

    writeColorPPM("terrain.ppm", heightmap, width, height);
    writeIsometricPPM("terrain_iso.ppm", heightmap, width, height);
    return 0;
}

