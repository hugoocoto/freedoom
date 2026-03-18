#ifndef ACTOR_H_
#define ACTOR_H_

#include "camera.h"
#include "mesh.h"
#include <cassert>

static void _before(Mesh *self);

class Actor : public Mesh
{
    private:
        Camera *camera;
        float camera_offset_x;
        float camera_offset_y;
        float camera_offset_z;
        bool jumping;
        float body_height;

    public:
        Actor(const char *name)
        : Mesh(name, 0x93ECFB),
          camera(nullptr),
          camera_offset_x(2.0f),
          camera_offset_y(1.0f),
          camera_offset_z(0.0f),
          jumping(false),
          body_height(0)
        {
                set_before_draw_function(_before);
        }

        void set_camera_yoffset(float y)
        {
                camera_offset_y = y;
        }

        void set_camera_zoffset(float z)
        {
                camera_offset_z = z;
        }


        float get_default_height()
        {
                if (body_height == 0)
                        body_height = get_absolute_position().y;
                return body_height;
        }

        bool is_bottom_colliding()
        {
                return get_absolute_position().y <= get_default_height();
        }

        void set_jumping(bool j)
        {
                jumping = j;
        }

        bool is_jumping()
        {
                return jumping;
        }

        float get_camera_yoffset()
        {
                return camera_offset_y;
        }

        void set_camera_xoffset(float x)
        {
                camera_offset_x = x;
        }

        float get_camera_xoffset()
        {
                return camera_offset_x;
        }

        void set_follow_camera(Camera *c)
        {
                camera = c;
                c->look_at(get_absolute_position());
        }

        void update_follow_camera()
        {
                if (camera == nullptr) return;

                /* for aimlabs */
                mat4 m = get_absolute_model();
                vec3 pos = get_absolute_position();
                vec3 dirf = normalize(vec3(m[2]));
                vec3 right = normalize(vec3(m[0])) * camera_offset_z;
                camera->look_at(pos + right);
                camera->place(pos + right + dirf * camera_offset_x + vec3(0.0f, camera_offset_y, 0.0f));
                camera->set_up(vec3(m[1]));
        }


        Camera *get_following_camera()
        {
                return camera;
        }

        void init()
        {
                Mesh::init();
                body_height = get_absolute_position().y;
        }
};

static void
_before(Mesh *self)
{
        ((Actor *) self)->update_follow_camera();
}

#endif // ACTOR_H_
