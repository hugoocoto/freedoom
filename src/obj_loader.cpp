#include "obj_loader.h"

#include "shape.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glm/geometric.hpp>
#include <glm/glm.hpp>

using glm::cross;
using glm::normalize;
using glm::vec2;
using glm::vec3;
using std::string;
using std::unordered_map;
using std::vector;

namespace {

struct FaceVertexIndex {
        int position = 0;
        int texcoord = 0;
        int normal = 0;
};

struct ObjMeshBuilder {
        string object_name;
        string name;
        string material_name;
        vector<float> vertices;
        vector<unsigned int> indices;

        bool empty() const
        {
                return indices.empty();
        }
};

int
resolve_obj_index(int index, size_t size)
{
        if (index > 0)
                return index - 1;
        if (index < 0)
                return static_cast<int>(size) + index;
        return -1;
}

FaceVertexIndex
parse_face_vertex(const string &token)
{
        FaceVertexIndex result;
        size_t first = token.find('/');

        if (first == string::npos) {
                result.position = std::stoi(token);
                return result;
        }

        string first_part = token.substr(0, first);
        if (!first_part.empty())
                result.position = std::stoi(first_part);

        size_t second = token.find('/', first + 1);
        if (second == string::npos) {
                string texcoord = token.substr(first + 1);
                if (!texcoord.empty())
                        result.texcoord = std::stoi(texcoord);
                return result;
        }

        string texcoord = token.substr(first + 1, second - first - 1);
        string normal = token.substr(second + 1);

        if (!texcoord.empty())
                result.texcoord = std::stoi(texcoord);
        if (!normal.empty())
                result.normal = std::stoi(normal);

        return result;
}

vec3
read_position(const vector<vec3> &positions, int raw_index)
{
        int index = resolve_obj_index(raw_index, positions.size());
        if (index < 0 || static_cast<size_t>(index) >= positions.size())
                return vec3(0.0f);
        return positions.at(index);
}

vec2
read_texcoord(const vector<vec2> &texcoords, int raw_index)
{
        int index = resolve_obj_index(raw_index, texcoords.size());
        if (index < 0 || static_cast<size_t>(index) >= texcoords.size())
                return vec2(0.0f);
        return texcoords.at(index);
}

vec3
read_normal(const vector<vec3> &normals, int raw_index)
{
        int index = resolve_obj_index(raw_index, normals.size());
        if (index < 0 || static_cast<size_t>(index) >= normals.size())
                return vec3(0.0f, 1.0f, 0.0f);
        return normals.at(index);
}

void
append_vertex(ObjMeshBuilder &mesh, vec3 position, vec2 texcoord, vec3 normal)
{
        mesh.vertices.push_back(position.x);
        mesh.vertices.push_back(position.y);
        mesh.vertices.push_back(position.z);
        mesh.vertices.push_back(texcoord.x);
        mesh.vertices.push_back(texcoord.y);
        mesh.vertices.push_back(normal.x);
        mesh.vertices.push_back(normal.y);
        mesh.vertices.push_back(normal.z);
        mesh.indices.push_back(static_cast<unsigned int>(mesh.indices.size()));
}

void
append_triangle(ObjMeshBuilder &mesh,
                const FaceVertexIndex &a,
                const FaceVertexIndex &b,
                const FaceVertexIndex &c,
                const vector<vec3> &positions,
                const vector<vec2> &texcoords,
                const vector<vec3> &normals)
{
        vec3 position_a = read_position(positions, a.position);
        vec3 position_b = read_position(positions, b.position);
        vec3 position_c = read_position(positions, c.position);

        bool has_normals = a.normal != 0 && b.normal != 0 && c.normal != 0;
        vec3 fallback_normal = normalize(cross(position_b - position_a, position_c - position_a));
        if (!glm::all(glm::isfinite(fallback_normal)) || glm::length(fallback_normal) == 0.0f)
                fallback_normal = vec3(0.0f, 1.0f, 0.0f);

        append_vertex(mesh,
                      position_a,
                      read_texcoord(texcoords, a.texcoord),
                      has_normals ? read_normal(normals, a.normal) : fallback_normal);
        append_vertex(mesh,
                      position_b,
                      read_texcoord(texcoords, b.texcoord),
                      has_normals ? read_normal(normals, b.normal) : fallback_normal);
        append_vertex(mesh,
                      position_c,
                      read_texcoord(texcoords, c.texcoord),
                      has_normals ? read_normal(normals, c.normal) : fallback_normal);
}

unordered_map<string, string>
load_materials(const std::filesystem::path &obj_path, const string &mtl_file)
{
        unordered_map<string, string> textures_by_material;
        std::ifstream input(obj_path.parent_path() / mtl_file);
        if (!input.is_open())
                return textures_by_material;

        string current_material;
        string line;
        while (std::getline(input, line)) {
                std::istringstream stream(line);
                string keyword;
                stream >> keyword;

                if (keyword == "newmtl") {
                        stream >> current_material;
                        continue;
                }

                if (keyword == "map_Kd" && !current_material.empty()) {
                        string texture_name;
                        std::getline(stream >> std::ws, texture_name);
                        if (!texture_name.empty()) {
                                textures_by_material[current_material] =
                                        (obj_path.parent_path() / texture_name).string();
                        }
                }
        }

        return textures_by_material;
}

void
finalize_mesh(const unordered_map<string, string> &textures_by_material,
              ObjMeshBuilder &builder,
              vector<ObjMeshData> &meshes)
{
        if (builder.empty())
                return;

        ObjMeshData mesh;
        mesh.name = builder.name.empty() ? "obj_mesh" : builder.name;
        if (!builder.material_name.empty()) {
                auto it = textures_by_material.find(builder.material_name);
                if (it != textures_by_material.end())
                        mesh.diffuse_texture = it->second;
        }

        gvopts opts = {};
        opts.vertex_start = 0;
        opts.vertex_coords = 3;
        opts.texture_start = 3;
        opts.texture_coords = 2;
        opts.normal_start = 5;
        opts.normal_coords = 3;
        opts.padd = 8;
        opts.use_vertex = true;
        opts.use_texture = true;
        opts.use_normal = true;

        GLuint index_count = static_cast<GLuint>(builder.indices.size());
        Shape::__gen_vao(&mesh.vao,
                         builder.vertices.size(),
                         builder.vertices.data(),
                         builder.indices.size(),
                         builder.indices.data(),
                         opts);
        mesh.index_count = index_count;
        meshes.push_back(std::move(mesh));

        builder.vertices.clear();
        builder.indices.clear();
}

void
refresh_mesh_name(ObjMeshBuilder &builder)
{
        builder.name = builder.object_name.empty() ? "obj_mesh" : builder.object_name;
        if (!builder.material_name.empty())
                builder.name += ":" + builder.material_name;
}

} // namespace

vector<ObjMeshData>
load_obj_meshes(const char *path)
{
        std::filesystem::path obj_path(path);
        std::ifstream input(obj_path);
        if (!input.is_open())
                return {};

        vector<vec3> positions;
        vector<vec2> texcoords;
        vector<vec3> normals;
        vector<ObjMeshData> meshes;
        unordered_map<string, string> textures_by_material;
        ObjMeshBuilder builder;
        builder.object_name = obj_path.stem().string();
        refresh_mesh_name(builder);

        string line;
        while (std::getline(input, line)) {
                if (line.empty() || line[0] == '#')
                        continue;

                std::istringstream stream(line);
                string keyword;
                stream >> keyword;

                if (keyword == "mtllib") {
                        string mtl_file;
                        std::getline(stream >> std::ws, mtl_file);
                        if (!mtl_file.empty())
                                textures_by_material = load_materials(obj_path, mtl_file);
                        continue;
                }

                if (keyword == "o" || keyword == "g") {
                        string name;
                        std::getline(stream >> std::ws, name);
                        finalize_mesh(textures_by_material, builder, meshes);
                        if (!name.empty())
                                builder.object_name = name;
                        refresh_mesh_name(builder);
                        continue;
                }

                if (keyword == "usemtl") {
                        string material_name;
                        std::getline(stream >> std::ws, material_name);
                        if (material_name != builder.material_name) {
                                finalize_mesh(textures_by_material, builder, meshes);
                                builder.material_name = material_name;
                                refresh_mesh_name(builder);
                        }
                        continue;
                }

                if (keyword == "v") {
                        vec3 position(0.0f);
                        stream >> position.x >> position.y >> position.z;
                        positions.push_back(position);
                        continue;
                }

                if (keyword == "vt") {
                        vec2 texcoord(0.0f);
                        stream >> texcoord.x >> texcoord.y;
                        texcoords.push_back(texcoord);
                        continue;
                }

                if (keyword == "vn") {
                        vec3 normal(0.0f);
                        stream >> normal.x >> normal.y >> normal.z;
                        normals.push_back(normal);
                        continue;
                }

                if (keyword == "f") {
                        vector<FaceVertexIndex> face;
                        string token;
                        while (stream >> token)
                                face.push_back(parse_face_vertex(token));

                        if (face.size() < 3)
                                continue;

                        for (size_t i = 1; i + 1 < face.size(); ++i)
                                append_triangle(builder, face[0], face[i], face[i + 1], positions, texcoords, normals);
                }
        }

        finalize_mesh(textures_by_material, builder, meshes);
        return meshes;
}
