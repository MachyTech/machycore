#ifndef _MACHY_GL_H_
#define _MACHY_GL_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <boost/asio.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

using boost::asio::steady_timer;

namespace machygl
{
    struct machygl_variables{
        std::string image_shader;
        bool image_shader_dirty;
        bool error_set;
        /* variable to hold our handle to the vertex array object */
        GLuint vao;
        /* buffer objects */
        GLuint buffer[2];
        /* active program */
        GLuint program;
        /* uniform values */
        float mvp[16];
        float timedelta;
        float time;
        float frame;
        GLfloat InnerColor[4];
        GLfloat OuterColor[4];
        GLfloat RadiusInner;
        GLfloat RadiusOuter;
        GLfloat resolution[2];
        /* locations of uniforms in the proram */
        GLuint mvp_location;
        GLuint InnerColor_location;
        GLuint OuterColor_location;
        GLuint RadiusInner_location;
        GLuint RadiusOuter_location;
        GLuint resolution_location;
        /* shaders data path */
        char *vertex_path, *fragment_path;
        /* widget ticker */
        long first_frame_time;
        long int first_frame;
        long int tick;
    };

    class Window
    {
        public:
            GLFWwindow* window;
            Window(std::string width, std::string height)
            {
                if (!glfwInit())
                    exit(EXIT_FAILURE);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

                std::string::size_type sz;

                window = glfwCreateWindow(std::stoi(width,&sz), std::stoi(height,&sz), "MachyTech", NULL, NULL);

                if (!window)
                {
                    glfwTerminate();
                    exit(EXIT_FAILURE);
                }

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                glfwMakeContextCurrent(window);
                gladLoadGL();
                glfwSwapInterval(1);
            }
            ~Window(void)
            {
                glfwDestroyWindow(window);
                glfwTerminate();
                exit(EXIT_SUCCESS);
            }
    };

    class utils
    {
        private:
            std::string vertex_shader_text;
            std::string fragment_shader_text;

            int vertex_flag, fragment_flag;

            const char* fs_text;
            const char* vs_text;

            long frame;

            std::vector<std::string> info;
            machygl_variables *machygl_var;
            GLFWwindow* win_;
            steady_timer frameticker_;
        public:
            utils(Window& win, boost::asio::io_context& io_context)
                : win_(win.window),
                    frameticker_(io_context),
                    frame(0),
                    machygl_var(new machygl::machygl_variables())
            {}
            GLuint vertex_shader, fragment_shader;
            void render();
            void set_image_shader(std::string);
            void realize();
            void realize_shader();
            void start(std::string);
            void tick();
            void compute_mvp(float *res, float phi, float theta, float psi);
            std::string read_shader(std::string direction);
            int get_compile_data(GLuint shader);
            GLuint link_shader_old(std::string vs_direction, std::string fs_direction);
            bool link_shader(std::string ver_src, std::string frag_src);
    };
}
#endif