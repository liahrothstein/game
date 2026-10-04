// GltfLoader.cpp
#include "assets/GltfLoader.h"
#include "cgltf.h"
#include <iostream>

bool GltfLoader::loadFirstMesh(const std::string& path, Mesh& out) {
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result r = cgltf_parse_file(&options, path.c_str(), &data);
    if (r != cgltf_result_success) {
        std::cerr << "gltf parse failed: " << path << "\n";
        return false;
    }
    r = cgltf_load_buffers(&options, data, path.c_str());   // подтягивает bin/GLB-данные
    if (r != cgltf_result_success) {
        std::cerr << "gltf load buffers failed\n";
        cgltf_free(data);
        return false;
    }
    if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
        std::cerr << "gltf: no meshes\n";
        cgltf_free(data);
        return false;
    }

    const cgltf_primitive& prim = data->meshes[0].primitives[0];

    // Читаем атрибуты: POSITION, NORMAL (если есть), TEXCOORD_0 (если есть)
    std::vector<Vertex> verts;
    const cgltf_accessor* aPos = nullptr, *aNorm = nullptr, *aUV = nullptr;
    for (cgltf_size i = 0; i < prim.attributes_count; ++i) {
        const cgltf_attribute& a = prim.attributes[i];
        if (a.type == cgltf_attribute_type_position)  aPos  = a.data;
        if (a.type == cgltf_attribute_type_normal)    aNorm  = a.data;
        if (a.type == cgltf_attribute_type_texcoord)  aUV    = a.data;
    }
    if (!aPos) { std::cerr << "gltf: no POSITION\n"; cgltf_free(data); return false; }

    // Читаем индексы
    std::vector<GLuint> indices(prim.indices->count);
    for (cgltf_size i = 0; i < prim.indices->count; ++i)
        indices[i] = (GLuint)cgltf_accessor_read_index(prim.indices, i);

    // Собираем вершины
    verts.resize(aPos->count);
    for (cgltf_size i = 0; i < aPos->count; ++i) {
        cgltf_accessor_read_float(aPos, i, &verts[i].pos.x, 3);
        if (aNorm) cgltf_accessor_read_float(aNorm, i, &verts[i].normal.x, 3);
        else       verts[i].normal = {0, 1, 0};
        if (aUV)   cgltf_accessor_read_float(aUV, i, &verts[i].uv.x, 2);
        else       verts[i].uv = {0, 0};
    }

    Mesh m;
    if (!m.build(verts, indices)) { cgltf_free(data); return false; }
    out = std::move(m);
    cgltf_free(data);
    return true;
}

bool GltfLoader::loadFirstImage(const std::string& path,
                                std::vector<unsigned char>& outData) {
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success
        || cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
        cgltf_free(data);
        return false;
    }

    bool ok = false;
    if (data->images_count > 0) {
        const cgltf_image& img = data->images[0];
        if (img.buffer_view) {   // GLB: картинка встроена в буфер
            const cgltf_buffer_view* bv = img.buffer_view;
            const unsigned char* src =
                (const unsigned char*)bv->buffer->data + bv->offset;
            outData.assign(src, src + bv->size);
            ok = true;
        }
        // (вариант «сайдкар-файл» с uri — добавим при необходимости)
    }
    cgltf_free(data);
    return ok;
}