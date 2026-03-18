#ifndef SHAPE_H_
#define SHAPE_H_

#include "camview.h"

#include "actor.h"
#include "camera.h"
#include "mesh.h"

#define PI 3.1416f
#define PIMED 1.5708f

#define Point(a, b, c) a, b, c
#define Face4(a, b, c, d) a, b, c, a, c, d
#define Face3(a, b, c) a, b, c
#define vec_ptr(vec, i) ((vec).data() + (i))
#define SIZE(arr) (sizeof((arr)) / sizeof(*(arr)))
#define Texture(a, b) a, b
#define Normal(a, b, c) a, b, c


struct gvopts {
        GLuint vertex_start, vertex_coords;
        GLuint texture_start, texture_coords;
        GLuint normal_start, normal_coords;
        GLuint padd;
        bool use_vertex, use_texture, use_normal;
};

class Shape
{
    public:
        Shape()
        {
        }
        ~Shape()
        {
        }

        static Mesh *crosshair(float)
        {
                Mesh *m = new Mesh("crosshair");
                GLuint vao, indexes_n;
                __crosshair_square(&vao, &indexes_n);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *hand(float)
        {
                Mesh *m = new Mesh("fullscreen");
                GLuint vao, indexes_n;
                __static_fullscreen(&vao, &indexes_n);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *plane(float l)
        {
                Mesh *m = new Mesh("plane");
                GLuint vao, indexes_n;
                __square(&vao, &indexes_n, l);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static CamView *camviewer_plane(float l)
        {
                CamView *p = new CamView("portal plane");
                GLuint vao, indexes_n;
                square_textured_vao(&vao, &indexes_n, l);
                p->set_vao(vao, indexes_n);
                return p;
        }

        static Mesh *cube(float l)
        {
                Mesh *m = new Mesh("cube");
                GLuint vao, indexes_n;
                cube_vao(&vao, &indexes_n, l, l, l);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *cube_nocollider(float x, float y, float z)
        {
                Mesh *m = new Mesh("cube no collider", 0xFF0000, true, false, false);
                GLuint vao, indexes_n;
                cube_vao(&vao, &indexes_n, x, y, z);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *cube_nocollider(float l)
        {
                Mesh *m = new Mesh("cube no collider", 0xFF0000, true, false, false);
                GLuint vao, indexes_n;
                cube_vao(&vao, &indexes_n, l, l, l);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Actor *actor(float l)
        {
                Actor *m = new Actor("Actor");
                GLuint vao, indexes_n;
                cube_vao(&vao, &indexes_n, l, l, l);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Actor *actor(float x, float y, float z)
        {
                Actor *m = new Actor("Actor");
                GLuint vao, indexes_n;
                cube_vao(&vao, &indexes_n, x, y, z);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *camera()
        {
                Mesh *m = new Mesh("Camera");
                GLuint vao, indexes_n;
                __camera(&vao, &indexes_n, 0.3f);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *
        line(vec3 from, vec3 to)
        {
                Mesh *m = new Mesh("line");
                GLuint vao, indexes_n;
                __line(&vao, &indexes_n, from, to);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *
        sphere(float r)
        {
                Mesh *m = new Mesh("sphere");
                GLuint vao, indexes_n;
                __sphere(&vao, &indexes_n, r);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static Mesh *
        sphere_collider(float r)
        {
                Mesh *m = new Mesh("sphere", 0xFF007F, true, false, false);
                GLuint vao, indexes_n;
                __sphere(&vao, &indexes_n, r);
                m->set_vao(vao, indexes_n);
                return m;
        }

        static void
        __gen_vao(GLuint *VAO, size_t n, const float *vertex, size_t m, const GLuint *indexes,
                  struct gvopts opts)
        {
                GLuint VBO, EBO;
                glGenVertexArrays(1, VAO);
                glBindVertexArray(*VAO);
                glGenBuffers(1, &VBO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, n * sizeof(*vertex), vertex, GL_STATIC_DRAW);
                glGenBuffers(1, &EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, m * sizeof(*indexes), indexes, GL_STATIC_DRAW);

                if (opts.use_vertex) {
                        glVertexAttribPointer(0, opts.vertex_coords, GL_FLOAT, GL_FALSE,
                                              opts.padd * sizeof(float), (void *) (sizeof(float) * opts.vertex_start));
                        glEnableVertexAttribArray(0);
                }
                if (opts.use_texture) {
                        glVertexAttribPointer(1, opts.texture_coords, GL_FLOAT, GL_FALSE,
                                              opts.padd * sizeof(float), (void *) (sizeof(float) * opts.texture_start));
                        glEnableVertexAttribArray(1);
                }
                if (opts.use_normal) {
                        glVertexAttribPointer(2, opts.normal_coords, GL_FLOAT, GL_FALSE,
                                              opts.padd * sizeof(float), (void *) (sizeof(float) * opts.normal_start));
                        glEnableVertexAttribArray(2);
                }

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                glDeleteBuffers(1, &EBO);
                glDeleteBuffers(1, &VBO);
        }

        static void
        __generateSphere(float radius, unsigned int sectorCount, unsigned int stackCount,
                         std::vector<float> &vertices, std::vector<unsigned int> &indices)
        {
                vertices.clear();
                indices.clear();

                for (unsigned int i = 0; i <= stackCount; ++i) {
                        float stackAngle = PI / 2 - i * (PI / stackCount);
                        float xy = radius * cosf(stackAngle);
                        float z = radius * sinf(stackAngle);

                        for (unsigned int j = 0; j <= sectorCount; ++j) {
                                float sectorAngle = j * (2 * PI / sectorCount);
                                float x = xy * cosf(sectorAngle);
                                float y = xy * sinf(sectorAngle);

                                vertices.push_back(x);
                                vertices.push_back(y);
                                vertices.push_back(z);
                        }
                }

                for (unsigned int i = 0; i < stackCount; ++i) {
                        for (unsigned int j = 0; j < sectorCount; ++j) {
                                unsigned int first = i * (sectorCount + 1) + j;
                                unsigned int second = first + sectorCount + 1;

                                indices.push_back(first);
                                indices.push_back(second);
                                indices.push_back(first + 1);

                                indices.push_back(second);
                                indices.push_back(second + 1);
                                indices.push_back(first + 1);
                        }
                }
        }

        static void
        __sphere(GLuint *VAO, GLuint *indexes_n, float radius)
        {
                vector<float> vertices;
                vector<unsigned int> indices;

                __generateSphere(radius, 10, 10, vertices, indices);
                *indexes_n = indices.size();

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.padd = 3;
                opts.use_vertex = true;
                opts.use_texture = false;
                opts.use_normal = false;

                __gen_vao(VAO, vertices.size(), vertices.data(), indices.size(), indices.data(), opts);
        }

        static void
        __static_fullscreen(GLuint *VAO, GLuint *indexes_n)
        {
                float size = 1.0f; // fullscreen
                float _x = size;

                float vertices[] = {
                        Point(-_x, _x, 0.0f),
                        Texture(0, 1),
                        Point(-_x, -_x, 0.0f),
                        Texture(0, 0),
                        Point(_x, -_x, 0.0f),
                        Texture(1, 0),
                        Point(_x, _x, 0.0f),
                        Texture(1, 1),
                };

                unsigned int indices[] = {
                        Face4(0, 1, 2, 3)
                };

                *indexes_n = SIZE(indices);

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.texture_start = 3;
                opts.texture_coords = 2;
                opts.padd = 5;
                opts.use_vertex = true;
                opts.use_texture = true;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }

        static void
        __crosshair_square(GLuint *VAO, GLuint *indexes_n)
        {
                float size = 0.2f; // Tamaño bien grande (ajústalo después)
                float _x = size / 2.0f;

                float vertices[] = {
                        Point(-_x, _x, 0.0f),
                        Texture(0, 1),
                        Point(-_x, -_x, 0.0f),
                        Texture(0, 0),
                        Point(_x, -_x, 0.0f),
                        Texture(1, 0),
                        Point(_x, _x, 0.0f),
                        Texture(1, 1),
                };

                unsigned int indices[] = {
                        Face4(0, 1, 2, 3)
                };

                *indexes_n = SIZE(indices);

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.texture_start = 3;
                opts.texture_coords = 2;
                opts.padd = 5;
                opts.use_vertex = true;
                opts.use_texture = true;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }


        static void
        __square(GLuint *VAO, GLuint *indexes_n, float x, float = 1, float = 1)
        {
                float _x = x / 2.0f;
                float vertices[] = {
                        Point(-_x, 0.0f, -_x), // Texture(0, 0),
                        Point(-_x, 0.0f, _x), // Texture(0, texture_scale * relation),
                        Point(_x, 0.0f, _x), // Texture(texture_scale * relation, texture_scale * relation),
                        Point(_x, 0.0f, -_x), // Texture(texture_scale * relation, 0),

                        /* 3 ---- 2
                         * |      |
                         * |      |
                         * 0 ---- 1 */
                };

                unsigned int indices[] = {
                        Face4(0, 1, 2, 3),
                        Face4(0, 3, 2, 1),
                };

                *indexes_n = SIZE(indices);

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.padd = 3;
                opts.use_vertex = true;
                opts.use_texture = false;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }

        static void
        square_textured_vao(GLuint *VAO, GLuint *indexes_n, float x, float texture_scale = 1, float relation = 1)
        {
                float _x = x / 2.0f;
                float vertices[] = {
                        Point(-_x, 0.0f, -_x), Texture(0, texture_scale * relation),
                        Point(-_x, 0.0f, _x), Texture(0, 0),
                        Point(_x, 0.0f, _x), Texture(texture_scale * relation, 0),
                        Point(_x, 0.0f, -_x), Texture(texture_scale * relation, texture_scale * relation),

                        /* 3 ---- 2
                         * |      |
                         * |      |
                         * 0 ---- 1 */
                };

                unsigned int indices[] = {
                        Face4(0, 1, 2, 3),
                };

                *indexes_n = SIZE(indices);

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.texture_start = 3;
                opts.texture_coords = 2;
                opts.padd = 5;
                opts.use_vertex = true;
                opts.use_texture = true;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }

        /* Create a cube with a given (x, y, z) size */
        static void
        __cube_textured(GLuint *VAO, GLuint *indexes_n, float x, float y, float z, float texture_scale = 1, float relation = 1)
        {
                /*                                       | y
             0(-x,y,-z)-> /---------/|<- (x,y,-z)4       |
                         / |       / |                   |______ x
                        /  |      /  |                  /
          1(-x,y,z)->  /_________/   |  <- (x,y,z)5    / z
         2(-x,-y,-z)-> |   /-----|---/ <- (x,-y,-z)6
                       |  /      |  /
                       | /       | /
          3(-x,-y,z)-> |/________|/ <- (x,-y,z)7


                */

                x /= 2;
                y /= 2;
                z /= 2;

                float vertices[] = {
                        Point(-x, y, -z), Texture(0, 0),
                        Point(-x, y, z), Texture(texture_scale * relation, 0),
                        Point(-x, -y, -z), Texture(texture_scale * relation, texture_scale * relation),
                        Point(-x, -y, z), Texture(0, texture_scale * relation),
                        Point(x, y, -z), Texture(0, texture_scale * relation),
                        Point(x, y, z), Texture(texture_scale * relation, texture_scale * relation),
                        Point(x, -y, -z), Texture(texture_scale * relation, 0),
                        Point(x, -y, z), Texture(0, 0)
                };

                GLuint indices[] = {
                        Face4(0, 1, 5, 4), // top
                        Face4(0, 2, 3, 1), // left
                        Face4(6, 4, 5, 7), // right
                        Face4(1, 3, 7, 5), // front
                        Face4(0, 4, 6, 2), // back
                        Face4(3, 2, 6, 7), // down
                };

                *indexes_n = SIZE(indices);
                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.texture_start = 3;
                opts.texture_coords = 2;
                opts.padd = 5;
                opts.use_vertex = true;
                opts.use_texture = true;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }

        /* Create a cube with a given (x, y, z) size */
        static void
        cube_vao(GLuint *VAO, GLuint *indexes_n, float x, float y, float z)
        {
                /*                                       | y
             0(-x,y,-z)-> /---------/|<- (x,y,-z)4       |
                         / |       / |                   |______ x
                        /  |      /  |                  /
          1(-x,y,z)->  /_________/   |  <- (x,y,z)5    / z
         2(-x,-y,-z)-> |   /-----|---/ <- (x,-y,-z)6
                       |  /      |  /
                       | /       | /
          3(-x,-y,z)-> |/________|/ <- (x,-y,z)7

                */

                x /= 2;
                y /= 2;
                z /= 2;

                float vertices[] = {
                        Point(-x, y, -z),
                        Point(-x, y, z),
                        Point(-x, -y, -z),
                        Point(-x, -y, z),
                        Point(x, y, -z),
                        Point(x, y, z),
                        Point(x, -y, -z),
                        Point(x, -y, z),
                };

                GLuint indices[] = {
                        Face4(0, 1, 5, 4), // top
                        Face4(0, 2, 3, 1), // left
                        Face4(6, 4, 5, 7), // right
                        Face4(1, 3, 7, 5), // front
                        Face4(0, 4, 6, 2), // back
                        Face4(3, 2, 6, 7), // down
                };

                *indexes_n = SIZE(indices);

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.padd = 3;
                opts.use_vertex = true;
                opts.use_texture = false;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }

        static void
        __line(GLuint *VAO, GLuint *indexes_n, vec3 from, vec3 to, float width = 0.1)
        {
                /*                                       | y
             0(-x,y,-z)-> /---------/|<- (x,y,-z)4       |
                         / |       / |                   |______ x
                        /  |      /  |                  /
          1(-x,y,z)->  /_________/   |  <- (x,y,z)5    / z
         2(-x,-y,-z)-> |   /-----|---/ <- (x,-y,-z)6
                       |  /      |  /
                       | /       | /
          3(-x,-y,z)-> |/________|/ <- (x,-y,z)7

                */

                float vertices[] = {
                        Point(from.x - width, from.y, from.z - width),
                        Point(from.x - width, from.y, from.z + width),
                        Point(to.x - width, to.y, to.z - width),
                        Point(to.x - width, to.y, to.z + width),
                        Point(from.x + width, from.y, from.z - width),
                        Point(from.x + width, from.y, from.z + width),
                        Point(to.x + width, to.y, to.z - width),
                        Point(to.x + width, to.y, to.z + width),
                };

                GLuint indices[] = {
                        Face4(0, 1, 5, 4), // top
                        Face4(0, 2, 3, 1), // left
                        Face4(6, 4, 5, 7), // right
                        Face4(1, 3, 7, 5), // front
                        Face4(0, 4, 6, 2), // back
                        Face4(3, 2, 6, 7), // down
                };

                *indexes_n = SIZE(indices);

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.padd = 3;
                opts.use_vertex = true;
                opts.use_texture = false;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }

        /* Create a cube with a given (x, y, z) size */
        static void
        __camera(GLuint *VAO, GLuint *indexes_n, float size = 0.0f)
        {
                float _size = size / 2.0f;
                float vertices[] = {
                        Point(-_size, _size, -_size), // 0
                        Point(-_size, _size, _size), // 1
                        Point(-_size, -_size, -_size), // 2
                        Point(-_size, -_size, _size), // 3
                        Point(_size, _size, -_size), // 4
                        Point(_size, _size, _size), // 5
                        Point(_size, -_size, -_size), // 6
                        Point(_size, -_size, _size), // 7
                        Point(0.0f, 0.0f, -_size), // 8
                        Point(_size, _size, -_size * 3), // 9
                        Point(-_size, _size, -_size * 3), // 10
                        Point(-_size, -_size, -_size * 3), // 11
                        Point(_size, -_size, -_size * 3), // 12
                };

                GLuint indices[] = {
                        Face4(0, 1, 5, 4), // top
                        Face4(0, 2, 3, 1), // left
                        Face4(6, 4, 5, 7), // right
                        Face4(1, 3, 7, 5), // front
                        Face4(0, 4, 6, 2), // back
                        Face4(3, 2, 6, 7), // down
                        Face4(10, 9, 12, 11), // camera front
                        Face3(8, 9, 10),
                        Face3(8, 10, 11),
                        Face3(11, 12, 8),
                        Face3(9, 8, 12),
                };

                *indexes_n = SIZE(indices);

                struct gvopts opts;
                opts.vertex_start = 0;
                opts.vertex_coords = 3;
                opts.padd = 3;
                opts.use_vertex = true;
                opts.use_texture = false;
                opts.use_normal = false;

                __gen_vao(VAO, SIZE(vertices), vertices, SIZE(indices), indices, opts);
        }
};

#endif // SHAPE_H_
