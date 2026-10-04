// GltfLoader.h
#pragma once
#include "render/Mesh.h"
#include <string>
#include <vector>

class GltfLoader {
public:
    // Читает первый mesh/primitive файла, конвертирует в наш Mesh
    static bool loadFirstMesh(const std::string& path, Mesh& out);
    static bool loadFirstImage(const std::string& path,
                               std::vector<unsigned char>& outData);
};