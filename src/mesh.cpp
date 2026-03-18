#include "mesh.h"
#include "camera.h"
#include "scene.h"
#include "settings.h"
#include "shape.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include "stb_image.h"

#include "../thirdparty/load_obj/load_obj.h"


vec3
SphereCollider::get_position()
{
        return parent->get_absolute_position();
}

void
SphereCollider::scale(vec3 v)
{
        radius *= std::max(v.x, std::max(v.y, v.z));
}

void
SphereCollider::set_sphere(float r)
{
        sphere = Shape::sphere_collider(r);
}

void
SphereCollider::draw(mat4 mat, int _do)
{
        if (active)
                // printf("Drawing sphere collider\n");
                sphere->draw(mat, _do);
}

vector<Mesh *>
Mesh::import_obj(const char *path)
{
        vector<Mesh *> attached_childs;
        vector<lObject> objs;
        objs = load_obj(path, LOAD_3_3);
        if (objs.size() <= 0) {
                printf("Can not load %s\n", path);
                return attached_childs;
        }
        lObject current = objs.back();
        vao = current.vao;
        indexes_n = current.index_n;
        // __shader = current.shader;
        // model = current.model;
        name = current.name; /* I can do that because current.name is set using strdup. */

        if (current.material && current.material->texture > 0)
                textures.push_back(current.material->texture);
        objs.pop_back();

        for (auto obj : objs) {
                Mesh *m = new Mesh(obj.name, color, printable, need_render, sphere_collider != nullptr);
                attached_childs.push_back(m);
                m->set_vao(obj.vao, obj.index_n);
                // m->set_model(current.model);
                m->set_scene(scene);
                obj.shader = __shader;
                m->set_shader(obj.shader);

                if (obj.material && obj.material->texture > 0)
                        m->add_texture(obj.material->texture);
                attach(m);
        }
        return attached_childs;
}

void
Mesh::show()
{
        // printf("Show mesh %s\n", get_name());
        printable = true;
}

void
Mesh::hide()
{
        // printf("Hide mesh %s\n", get_name());
        printable = false;
}

void
Mesh::set_vao(GLuint _vao, GLuint _indexes_n)
{
        vao = _vao;
        indexes_n = _indexes_n;
}

void
Mesh::set_shader(GLuint shader)
{
        __shader = shader;
        if (sphere_collider && sphere_collider->is_pintable())
                sphere_collider->get_sphere()->set_shader(shader);
        for (auto c : attached)
                c->set_shader(shader);
}

const char *
Mesh::get_name()
{
        return name;
}

vec3
Mesh::get_position()
{
        return vec3(model[3]);
}

mat4
Mesh::get_absolute_model()
{
        if (parent) {
                return parent->get_absolute_model() * model;
        }
        return model;
}

mat4
Mesh::get_default_absolute_model()
{
        if (parent) {
                return parent->get_default_absolute_model() * default_model;
        }
        return default_model;
}

vec3
Mesh::get_absolute_position()
{
        return vec3(get_absolute_model()[3]);
}

vec3
Mesh::get_default_absolute_position()
{
        return vec3(get_default_absolute_model()[3]);
}

mat3
Mesh::get_rotation_matrix()
{
        return mat3(model);
}

vec3
Mesh::get_rotation()
{
        return eulerAngles(quat_cast(mat3(model)));
}

mat3
Mesh::get_default_rotation_matrix()
{
        return mat3(default_model);
}

mat4
Mesh::get_model()
{
        return model;
}

void
Mesh::set_model(mat4 _model)
{
        model = _model;
}

void
Mesh::set_before_draw_function(void (*_before_draw)(Mesh *))
{
        before_draw = _before_draw;
}

void
Mesh::set_color(int c)
{
        color = c;
}

unsigned int
Mesh::get_shader()
{
        return __shader;
}

#include <queue>
void
Mesh::scale(vec3 v)
{
        model = glm::scale(model, v);
        if (sphere_collider)
                sphere_collider->scale(v);
        for (auto m : attached) {
                if (m->get_sphere_collider())
                        m->get_sphere_collider()->scale(v);
        }
}

void
Mesh::scale(float v)
{
        this->scale(vec3(v, v, v));
}

void
Mesh::rotate(float angle, vec3 v)
{
        model = glm::rotate(model, angle, v);
}

void
Mesh::place(vec3 v)
{
        mat4 m = get_model();
        m[3] = vec4(v, 1.0f);
        set_model(m);
}

void
Mesh::move(vec3 v)
{
        vec3 pos = get_position() + v;
        mat4 m = get_model();
        m[3] = vec4(pos, 1.0f);
        set_model(m);
}

void
Mesh::translate(vec3 v)
{
        model = glm::translate(model, v);
}

void
Mesh::look_at2d(vec3 view_pos)
{
        vec3 mesh_pos = get_position();
        mat3 rotation = get_default_rotation_matrix();
        vec3 dir = normalize(vec3(view_pos.x - mesh_pos.x, 0.0, view_pos.z - mesh_pos.z));
        float angle = atan2(dir.x, dir.z);
        mat4 _model = mat4(1.0f);
        _model = glm::translate(_model, mesh_pos);
        _model = glm::rotate(_model, angle, vec3(0.0, 1.0, 0.0));
        _model *= mat4(rotation);
        model = _model;
}

void
Mesh::look_at(vec3 view_pos)
{
        mat4 mesh_model = get_absolute_model();
        vec3 pos = get_position();
        vec3 up = mesh_model[1];
        model = inverse(lookAt(get_absolute_position(), view_pos, up));
        place(pos);
}

void
Mesh::add_texture(GLuint texture)
{
        textures.push_back(texture);
}

void
Mesh::draw(mat4 _model, int _do)
{
        _model = _model * model;

        for (Mesh *attached_mesh : attached) {
                // printf("attached draw: %s attach to %s\n", attached_mesh->get_name(), get_name());
                // printf("pos: %f,%f,%f\n",
                //                 attached_mesh->get_position().x,
                //                 attached_mesh->get_position().y,
                //                 attached_mesh->get_position().z
                //                 );
                (*attached_mesh).draw(_model, _do);
        }

        if (sphere_collider && sphere_collider->is_pintable() && draw_collision_sphere)
                sphere_collider->draw(_model);


        glUseProgram(__shader);
        glEnable(GL_DEPTH_TEST);

        if (before_draw) before_draw(this);
        if (!printable) return;

        // printf("Drawing: %s\n", get_name());

        ((Scene *) scene)->get_camera()->set_camera(__shader);

        /* Set texture */
        GLuint textureLoc = glGetUniformLocation(__shader, "textures");
        glUniform1i(glGetUniformLocation(__shader, "texture_count"), textures.size());
        for (size_t i = 0; i < textures.size(); ++i) {
                GLuint texture = textures.at(i);
                if (glIsTexture(texture)) {
                        // printf("Using texture %d for model %s\n", texture, get_name());
                        glActiveTexture(GL_TEXTURE0 + i);
                        textureLoc = glGetUniformLocation(__shader, ("textures[" + std::to_string(i) + "]").c_str());
                        glUniform1i(textureLoc, i);
                        glBindTexture(GL_TEXTURE_2D, texture);

                } else if (texture != 0) {
                        printf("Texture %d failed!\n", texture);
                        continue;
                }
        }

        glUniformMatrix4fv(glGetUniformLocation(__shader, "model"), 1, GL_FALSE, value_ptr(_model));

        if (is_selected)
                _do |= DRAW_OUTLINE;

        if (_do & DRAW_OUTLINE) {
                glUniform3f(glGetUniformLocation(__shader, "color"), HexColor(0xFFDE21));
                glLineWidth(10);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glCullFace(GL_FRONT);
                glBindVertexArray(vao);
                glDrawElements(GL_TRIANGLES, indexes_n, GL_UNSIGNED_INT, 0);
                glCullFace(GL_BACK);
        }

        if (_do & DRAW_FILL) {
                glDisable(GL_CULL_FACE);
                glUniform3f(glGetUniformLocation(__shader, "color"), HexColor(color));
                glBindVertexArray(vao);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawElements(GL_TRIANGLES, indexes_n, GL_UNSIGNED_INT, 0);
        }

        /* Print lines above filled faces */
        if (_do & DRAW_LINE && _do & ~DRAW_OUTLINE) {
                glLineWidth(1);
                glUniform3f(glGetUniformLocation(__shader, "color"), HexColor(0x000000));
                glBindVertexArray(vao);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawElements(GL_TRIANGLES, indexes_n, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
}

void
Mesh::attach(Mesh *child)
{
        if (child->parent) return;
        child->parent = this;
        assert(scene && "Parent has no scene while attaching!\n");
        child->scene = scene;
        if (child->get_sphere_collider())
                child->get_sphere_collider()->get_sphere()->set_scene(scene);
        attached.push_back(child);
}

void
Mesh::set_before_draw(Mesh *mesh, void (*func)(Mesh *))
{
        mesh->before_draw = func;
}

bool
Mesh::is_attached()
{
        return parent != nullptr;
}

void
Mesh::set_scene(void *s)
{
        scene = s;
        if (sphere_collider && sphere_collider->is_pintable())
                sphere_collider->get_sphere()->set_scene(s);
        for (auto c : attached)
                c->set_scene(s);
}

void *
Mesh::get_scene()
{
        return scene;
}

void
Mesh::delete_vao()
{
        glDeleteVertexArrays(1, &vao);
}

void
Mesh::set_dynamic_camera(int c)
{
        dynamic_camera = c;
}

SphereCollider *
Mesh::get_sphere_collider()
{
        return sphere_collider;
}

void
Mesh::init()
{
        default_model = model;
}

void
Mesh::add_texture_image(const char *path, int how)
{
        int height, width, comp;
        unsigned char *image;
        unsigned int texture;

        stbi_set_flip_vertically_on_load(1);
        image = stbi_load(path, &width, &height, &comp, STBI_rgb_alpha);

        if (image == NULL) {
                fprintf(stderr, "Failed to load texture '%s'\n", path);
                return;
        }

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, how != 0 ? how : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, how != 0 ? how : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        assert(width > 0 && height > 0);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        assert(texture > 0);
        textures.push_back(texture);
        stbi_image_free(image);
}
