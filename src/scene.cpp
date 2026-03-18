#include "scene.h"
#include "camera.h"
#include "camview.h"
#include "mesh.h"
#include "panic.h"
#include "shape.h"

#include "setShaders.h"
#include <cstdio>
#include <glm/geometric.hpp>
#include <queue>

Camera *
Scene::get_camera(int index)
{
        if (index < 0)
                index = camera_index;

        if (static_cast<size_t>(index) >= cameras.size())
                PANIC("Can not locate camera");

        return cameras.at(index);
}

int
IntersectRaySphere(vec3 p, vec3 d, SphereCollider *sphere, float &t, vec3 &q)
{
        if (sphere == nullptr) {
                return 0;
        }

        vec3 center = sphere->get_position();
        float radius = sphere->get_radius();

        if (radius <= 0)
                return 0;

        vec3 m = p - center;
        float b = dot(m, d);
        float c = dot(m, m) - radius * radius;

        // Salida temprana si el rayo parte fuera y se aleja de la esfera
        if (c > 0.0f && b > 0.0f) {
                return 0;
        }

        float discr = b * b - c;

        // Discriminante negativo → sin intersección
        if (discr < 0.0f) {
                return 0;
        }

        // Se calcula el t más pequeño (entrada)
        float t_hit = -b - sqrt(discr);

        // Si el rayo parte desde dentro, usar t = 0
        if (t_hit < 0.0f) t_hit = 0.0f;

        // Resultado final
        t = t_hit;
        q = p + t * d;

        return 1;
}

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

void
Scene::repr_audio(const char *filename)
{
        static bool audio_initialized = false;
        static ma_engine engine;

        if (!audio_initialized) {
                if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
                        printf("Can not start audio engine!\n");
                        return;
                }
                audio_initialized = true;
        }

        if (filename && ma_engine_play_sound(&engine, filename, NULL) != MA_SUCCESS) {
                printf("Can not play sound: %s\n", filename);
        }
}

Mesh *
Scene::get_raycast_collision(vec2 mouse)
{
        Camera *camera = get_camera();

        // world_point = inverse(view) * inverse(projection) * mouse_point;

        vec4 from = vec4(camera->get_position(), 1.0f);
        vec4 to = inverse(camera->get_view()) * inverse(camera->get_projection()) * vec4(mouse, 1, 1);

        from /= from.w; // perspective divide ("normalize" homogeneous coordinates)
        to /= to.w;

        /* Represent ray */
        // Mesh *l = Shape::line(from, to);
        // l->set_shader(meshes.at(0)->get_shader()); // TODO
        // l->set_scene(this);
        // meshes.push_back(l);


        int clickedObject = -1;
        float minDist = 10000.0f;
        vec3 _q; // collision point

        vector<Mesh *> __m;
        queue<Mesh *> pending;

        for (auto *m : meshes) {
                __m.push_back(m);
                pending.push(m);
        }

        while (!pending.empty()) {
                Mesh *current = pending.front();
                pending.pop();

                for (auto *attached : current->get_attached_meshes()) {
                        __m.push_back(attached);
                        pending.push(attached);
                }
        }

        for (size_t i = 0; i < __m.size(); ++i) {
                float t; // collision distance
                vec3 q; // collision point
                vec3 direction = normalize(vec3(to) - vec3(from));
                if (IntersectRaySphere(camera->get_position(), direction, __m.at(i)->get_sphere_collider(), t, q)) {
                        // object i has been clicked. probably best to find the minimum t1 (front-most object)

                        if (t < minDist) {
                                minDist = t;
                                _q = q;
                                clickedObject = (int) i;
                        }
                }
        }

        if (clickedObject >= 0) {
                // printf("Click on %s\n", __m.at(clickedObject)->get_name());
                __m.at(clickedObject)->get_sphere_collider()->on_collision();

                /* Represent collision point */
                if (show_collision_point) {
                        Mesh *c = Shape::cube_nocollider(0.1f);
                        c->set_shader(__m.at(0)->get_shader()); // TODO
                        c->set_scene(this);
                        __m.push_back(c);
                        c->translate(_q);
                }

                // repr_audio("assets/pistol.wav");
                return __m.at(clickedObject);
        }

        return nullptr;
}

void
Scene::set_on_collision(void (*_on_collide)(SphereCollider *))
{
        for (auto m : meshes) {
                if (m->get_sphere_collider())
                        m->get_sphere_collider()->set_on_collide(_on_collide);
        }
}

int
Scene::get_camera_id()
{
        return camera_index;
}

void
Scene::set_camera(GLuint index)
{
        Camera *c = cameras.at(camera_index);
        c->get_mesh()->show();
        c->get_mesh()->enable_sphere_collider();

        if (index < cameras.size()) {
                camera_index = index;
        }

        c = cameras.at(camera_index);
        c->get_mesh()->hide();
        c->get_mesh()->disable_sphere_collider();
}

int
Scene::add_camera(Camera *c)
{
        cameras.push_back(c);
        if (c->get_mesh())
                add_mesh(c->get_mesh());
        return cameras.size() - 1;
}

void
Scene::add_mesh(Mesh *m)
{
        m->set_scene(this);
        meshes.push_back(m);
}

/* Render render-textures */
void
Scene::render()
{
        for (auto m : meshes)
                if (m->need_render) {
                        /* No se si render recursion funciona asi. La idea es que se
                         * vea un camview a traves del otro pero solo RENDER_RECURSION
                         * saltos. Si se activa hide - show entonces no aparece nunca. */
                        int render_recursion = 10;
                        for (int i = 0; i < render_recursion; i++) {
                                // m->hide();
                                ((CamView *) m)->render();
                                // m->show();
                        }
                }
}

void
Scene::draw()
{
        if (skybox)
                skybox->draw();
        for (auto m : meshes) {
                m->draw();
        }
}

void
Scene::init()
{
        repr_audio(NULL); // init engine
        for (auto m : meshes)
                m->init();
}


void
Scene::use_shader(const char *vs, const char *fs)
{
        GLuint shader = setShaders(vs, fs);
        for (auto m : meshes)
                m->set_shader(shader);
}

void
Scene::next_camera()
{
        camera_index = (camera_index + 1) % cameras.size();
        // printf("Camera index: %d\n", camera_index);
}
