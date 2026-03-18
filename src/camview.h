#ifndef CAM_VIEW_H_
#define CAM_VIEW_H_

#include "camera.h"
#include "mesh.h"

#include <cassert>
#include <glad/glad.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string.h>

#include "scene.h"

class CamView : public Mesh
{
    private:
        GLuint render_texture;
        GLuint deph_buffer;
        GLuint frame_buffer;
        int width, heigth;
        int prev_vp[4];
        int prev_fb;
        int prev_cam;

        void create_framebuffer()
        {
                glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_fb);
                glGenFramebuffers(1, &frame_buffer);
                assert(frame_buffer > 0);
                glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        }

        void gen_texture()
        {
                glGenTextures(1, &render_texture);
                glBindTexture(GL_TEXTURE_2D, render_texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                // glBindTexture(GL_TEXTURE_2D, 0);
                printf("Adding render_texture\n");
                add_texture(render_texture);
        }

        void gen_depth_buffer()
        {
                glGenRenderbuffers(1, &deph_buffer);
                glBindRenderbuffer(GL_RENDERBUFFER, deph_buffer);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, heigth);
        }

        void config_framebuffer()
        {
                glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, deph_buffer);
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render_texture, 0);
                GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
                glDrawBuffers(1, drawBuffers);
                assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
                glBindFramebuffer(GL_FRAMEBUFFER, prev_fb);
        }

    public:
        CamView(const char *name)
        : Mesh(name, 0xFF07E6, true, true),
          render_texture(0),
          deph_buffer(0),
          frame_buffer(0),
          width(0),
          heigth(0),
          prev_fb(0)
        {
                memset(prev_vp, 0, sizeof(prev_vp));
                printf("Calling camview constructor\n");
        }

        void config(int h = 0, int w = 0)
        {
                width = (w != 0) ? w : 800;
                heigth = (h != 0) ? h : 800;
                create_framebuffer();
                gen_texture();
                gen_depth_buffer();
                config_framebuffer();
        }

        void bind()
        {
                glGetIntegerv(GL_VIEWPORT, prev_vp);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);

                glViewport(0, 0, width, heigth);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                if (dynamic_camera > 0) {
                        prev_cam = ((Scene *) scene)->get_camera_id();
                        ((Scene *) get_scene())->set_camera(dynamic_camera);
                }
        }

        void unbind()
        {
                glBindTexture(GL_TEXTURE_2D, render_texture);
                glGenerateMipmap(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_fb);
                glViewport(prev_vp[0], prev_vp[1], prev_vp[2], prev_vp[3]);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                if (dynamic_camera > 0) {
                        ((Scene *) scene)->set_camera(prev_cam);
                }
        }

        void render()
        {
                bind();
                ((Scene *) get_scene())->draw();
                unbind();
        }
};

#endif // CAM_VIEW_H_
