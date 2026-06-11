# geometric-modeling

Used the version from Iakov given in discord the 07/04/2026 in the discord in the ressoources channel. 
Prior to this I was using my own version and made AI corrections to make this version work on my mac but had way to much problems so I switched over to Iakov’s.

### Disclaimer AI Usage 

I used AI in this project mostly for problem solving such as bug related to compiling the code (zsh segmentations errors).
When the programs was going through the mesh and going out of bounds most of the time. 
As well as to understand some concepts which I didn't get in class (such as surface of revolution). 

A part of triangulate was generated for normals 


# Mesh Viewer (OpenGL 3.3 + SDL2)

Read me from Iakov for the base project. 

Cross-platform CMake build (Windows, Linux, macOS) with modern OpenGL rendering/picking and SDL2 window/input layer.

## Build Requirements

- CMake 3.20+
- C++17 compiler
- OpenGL
- Git (for `FetchContent` dependencies)
- Internet access during first configure

## Build

Same commands on Windows, Linux, and macOS:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

The executable is named `meshviewer`.


## Reporting 

All implementations done on this project 

```cpp
myMesh.cpp : readFile(std::string filename)

myMesh.cpp : computeNormals()
myVertex.cpp : computeNormal()
myFace.cpp : computeNormal()

main.cpp : if (drawsilouette)

myMesh.cpp : triangulate() and bool triangulate() 

myMesh.cpp : splitFaceTRIS(myFace *f, myPoint3D *p) 

myMesh.cpp : splitEdge(myHalfedge *e1, myPoint3D *p) 

myMesh.cpp : splitFaceQUADS(myFace *f, myPoint3D *p) 
```