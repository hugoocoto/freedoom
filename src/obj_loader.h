#ifndef OBJ_LOADER_H_
#define OBJ_LOADER_H_

#include <glad/glad.h>

#include <string>
#include <vector>

struct ObjMeshData {
        std::string name;
        GLuint vao = 0;
        GLuint index_count = 0;
        std::string diffuse_texture;
};

std::vector<ObjMeshData> load_obj_meshes(const char *path);

#endif // OBJ_LOADER_H_
