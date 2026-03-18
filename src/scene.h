#ifndef SCENE_H_
#define SCENE_H_

#include <complex>
#include <glm/ext/vector_float2.hpp>
#include <glm/geometric.hpp>
#include <utility>
#include <vector>

#include "actor.h"
#include "camera.h"
#include "mesh.h"
#include "skybox.h"

#include <GLFW/glfw3.h>

#ifndef GLFW_CURSOR_CAPTURED
#define GLFW_CURSOR_CAPTURED GLFW_CURSOR_DISABLED
#endif

#include "settings.h"

inline bool locks[1024] = { 0 };
#define oneclick(__window, __key)                                            \
                glfwGetKey(__window, __key) == GLFW_RELEASE && locks[__key]) \
        {                                                                    \
                locks[__key] = false;                                        \
        }                                                                    \
        if (glfwGetKey(__window, __key) == GLFW_PRESS &&!locks[__key] &&(locks[__key]= true)

class Scene;
static void cam_movement_input_handler(GLFWwindow *window, Scene *scene);


class Scene
{
    private:
        vector<Camera *> cameras;
        vector<Mesh *> meshes;
        int camera_index;
        void (*_input_handler)(GLFWwindow *, Scene *);
        Actor *actor; // todo: handle actors in a proper way
        Skybox *skybox;

    public:
        Scene()
        : camera_index(0),
          _input_handler(cam_movement_input_handler),
          actor(nullptr),
          skybox(nullptr)
        {
        }
        ~Scene()
        {
        }

        void create_skybox(std::vector<std::string> faces)
        {
                skybox = new Skybox();
                skybox->set_scene(this);
                skybox->add_cubemap_image(faces);
                skybox->init();
        }

        void repr_audio(const char *filename);
        Camera *get_camera(int index = -1);
        int get_camera_id();
        int add_camera(Camera *c);
        void add_mesh(Mesh *m);
        void draw();
        void init();
        void use_shader(const char *, const char *);
        void next_camera();
        void render();
        Mesh *get_raycast_collision(vec2 mouse);
        void set_on_collision(void (*_on_collide)(SphereCollider *));


        void (*get_default_handler())(GLFWwindow *, Scene *)
        {
                return cam_movement_input_handler;
        }

        void set_handler(void (*handler)(GLFWwindow *, Scene *))
        {
                _input_handler = handler;
        }

        void process_input(GLFWwindow *window)
        {
                _input_handler(window, this);
        }

        void unselect_all()
        {
                for (auto m : meshes) {
                        if (m == actor)
                                continue;
                        if (m->selected())
                                m->deselect();
                }
        }

        void add_actor(Actor *a)
        {
                actor = a;
                add_mesh(a);
        }

        Actor *get_actor()
        {
                return actor;
        }

        void set_camera(Camera *c)
        {
                for (size_t i = 0; i < cameras.size(); i++) {
                        if (c == cameras.at(i)) {
                                set_camera(static_cast<GLuint>(i));
                                return;
                        }
                }
        }

        void set_on_select(void (*func)(Mesh *))
        {
                for (auto m : meshes)
                        m->set_on_select(func);
        }

        void set_on_deselect(void (*func)(Mesh *))
        {
                for (auto m : meshes)
                        m->set_on_deselect(func);
        }

        void set_camera(GLuint);
};

/* From main as it should be a class that control state
 * to handle all this stuff but there isnt. */
extern double *interframe_time();
extern vec2 get_window_size();
inline int mouse_mode = -1;

static void
cam_movement_input_handler(GLFWwindow *window, Scene *scene)
{
        /* Esto no deberia ir aqui */
        if (mouse_mode != GLFW_CURSOR_CAPTURED) {
                mouse_mode = GLFW_CURSOR_CAPTURED;

                /* glfw for windows dont have this */
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#else
                if (glfwRawMouseMotionSupported())
#endif

                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        }

        static bool initialized = false;
        static float xprev, yprev;
        double _xpos, _ypos;

        glfwGetCursorPos(window, &_xpos, &_ypos);
        float xpos = static_cast<float>(_xpos);
        float ypos = static_cast<float>(_ypos);

        if (!initialized) {
                xprev = xpos;
                yprev = ypos;
                initialized = true;
        }

        float xoffset = -xpos + xprev;
        float yoffset = ypos - yprev;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                scene->get_camera()->axis_rotate(vec3(MOUSE_SENS_X * xoffset * MOUSE_REDUCTION,
                                                      MOUSE_SENS_Y * yoffset * MOUSE_REDUCTION * 10.0f,
                                                      0.0f));
        }

        static bool lock_l = false;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !lock_l) {
                /* The *2.0f - 1.0 is to transform from [0,1] to [-1, 1] */
                vec2 mouse = (vec2(xpos, ypos) / get_window_size()) * 2.0f - 1.0f;
                mouse.y = -mouse.y; // origin is top-left and +y mouse is down
                scene->get_raycast_collision(mouse);
                lock_l = true;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && lock_l) {
                lock_l = false;
        }

        xprev = xpos;
        yprev = ypos;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

        vec3 dirf = -vec3(0, 0, 1);
        vec3 right = vec3(1, 0, 0);
        vec3 movement = vec3(0.0f);

        // Movimiento
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                movement += dirf;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                movement -= dirf;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                movement -= right;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                movement += right;

        double speed = 2.0f;

        if (movement != vec3(0))
                movement = normalize(movement) * static_cast<float>(*interframe_time() * speed);
        // printf("movement: %f,%f,%f\n", movement.x, movement.y, movement.z);

        scene->get_camera()->transform(movement);

        if (oneclick(window, GLFW_KEY_T)) {
                scene->next_camera();
        }
}

#endif // SCENE_H_
