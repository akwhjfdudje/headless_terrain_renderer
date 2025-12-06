# Headless Terrain Renderer
This repository contains a terrain renderer that directly outputs images of terrain.

This serves as a proof-of-concept of an application of some of my own CUDA kernels.

## Building

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
