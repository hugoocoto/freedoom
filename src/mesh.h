#ifndef MESH_H_
#define MESH_H_

#define HexColor(hex_color)                     \
        ((hex_color & 0xFF0000) >> 16) / 256.0, \
        ((hex_color & 0xFF00) >> 8) / 256.0,    \
        ((hex_color & 0xFF)) / 256.0

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cstdio>
#include <glm/glm.hpp>
#include <vector>

using namespace glm;
using namespace std;

enum draw_opts {
        DRAW_NONE = 0,
        DRAW_LINE = 1 << 1,
        DRAW_FILL = 1 << 2,
        DRAW_OUTLINE = 1 << 3,
};

class Mesh;
class SphereCollider;

typedef void (*on_collide_func)(SphereCollider *);

class SphereCollider
{
    private:
        float radius;
        Mesh *parent;
        bool active;
        Mesh *sphere;
        vector<on_collide_func> on_collide;

    public:
        SphereCollider(float r, Mesh *p)
        : radius(r),
          parent(p),
          active(true),
          sphere(nullptr)
        {
                set_sphere(radius);
        }

        void draw(mat4 mat, int _do = DRAW_LINE);

        float get_radius()
        {
                if (active)
                        return radius;
                return 0;
        }

        bool is_pintable()
        {
                return sphere != nullptr && active;
        }

        Mesh *get_sphere()
        {
                if (active)
                        return sphere;
                return nullptr;
        }

        void set_active(bool a)
        {
                active = a;
        }

        void set_on_collide(void (*_on_collide)(SphereCollider *))
        {
                on_collide.push_back(_on_collide);
        }

        void on_collision()
        {
                if (active == false)
                        return;

                if (on_collide.size() > 0)
                        for (auto func : on_collide)
                                func(this);
        }

        Mesh *get_parent()
        {
                return parent;
        }

        void scale(vec3 v);
        void set_sphere(float r);
        vec3 get_position();
};

class Mesh
{
    public:
        const bool need_render;

    private:
        const char *name;
        GLuint vao;
        GLuint indexes_n;
        bool printable;
        int color;
        mat4 model; // relative model
        mat4 default_model; // default starting relative model
        vector<Mesh *> attached;
        vector<GLuint> textures;
        Mesh *parent;
        unsigned int __shader;
        void (*before_draw)(Mesh *);
        SphereCollider *sphere_collider;
        bool is_selected;
        void (*input_handler)(GLFWwindow *);
        vector<void (*)(Mesh *)> on_selected;
        vector<void (*)(Mesh *)> on_deselected;
        void *attached_camera;

    protected:
        void *scene;
        int dynamic_camera;

    public:
        Mesh(const char *name, int _color = 0xFFFFFF, bool _printable = true, bool render = false, bool has_collider = true, float collider_radius = 0.7f)
        : need_render(render),
          name(name),
          vao(0),
          indexes_n(0),
          printable(_printable),
          color(_color),
          model(mat4(1.0f)),
          default_model(mat4(1.0f)),
          attached(),
          parent(nullptr),
          __shader(0),
          before_draw(nullptr),
          sphere_collider(nullptr),
          is_selected(false),
          input_handler(nullptr),
          attached_camera(nullptr),
          scene(nullptr),
          dynamic_camera(-1)
        {
                if (has_collider) {
                        // printf("Creating collider sphere with r=%f for mesh %s\n", collider_radius, name);
                        sphere_collider = new SphereCollider(collider_radius, this);
                }
        }

        vector<Mesh *> import_obj(const char *path);
        void move(vec3 v);
        void set_vao(GLuint _vao, GLuint _indexes_n);
        void show();
        void hide();
        void set_shader(GLuint shader);
        const char *get_name();
        vec3 get_position();
        mat4 get_absolute_model();
        vec3 get_absolute_position();
        mat4 get_default_absolute_model();
        vec3 get_default_absolute_position();
        mat3 get_rotation_matrix();
        vec3 get_rotation();
        mat3 get_default_rotation_matrix();
        mat4 get_model();
        void set_model(mat4 _model);
        void set_before_draw_function(void (*_before_draw)(Mesh *));
        unsigned int get_shader();
        void rotate(float angle, vec3 v);
        void scale(vec3 v);
        void scale(float v);
        void translate(vec3 v);
        void look_at2d(vec3 view_pos);
        void look_at(vec3 view_pos);
        void draw(mat4 _model = mat4(1.0f), int _do = DRAW_LINE | DRAW_FILL);
        void attach(Mesh *child);
        void set_before_draw(Mesh *mesh, void (*func)(Mesh *));
        bool is_attached();
        void delete_vao();
        void set_scene(void *);
        void set_color(int);
        void init();
        void set_dynamic_camera(int);
        void *get_scene();
        void add_texture(GLuint);
        SphereCollider *get_sphere_collider();
        void select()
        {
                is_selected = true;
                for (auto func : on_selected)
                        func(this);
        }
        void deselect()
        {
                is_selected = false;
                for (auto func : on_deselected)
                        func(this);
        }
        bool selected()
        {
                return is_selected;
        }

        vector<Mesh *> get_attached_meshes()
        {
                return attached;
        }

        void set_on_select(void (*func)(Mesh *))
        {
                on_selected.push_back(func);
        }

        void set_on_deselect(void (*func)(Mesh *))
        {
                on_deselected.push_back(func);
        }

        void place(vec3 v);

        void *get_attached_camera()
        {
                return attached_camera;
        }

        void attach_camera(void *c)
        {
                attached_camera = c;
        }

        void set_input_handler(void (*_input_handler)(GLFWwindow *))
        {
                input_handler = _input_handler;
        }

        void (*get_input_handler())(GLFWwindow *)
        {
                return input_handler;
        }

        void enable_sphere_collider()
        {
                if (sphere_collider)
                        sphere_collider->set_active(true);
        }

        void disable_sphere_collider()
        {
                if (sphere_collider)
                        sphere_collider->set_active(false);
        }

        void add_texture_image(const char *path, int how = 0);
};

#endif
