#ifndef _MACHY_GL_H_
#define _MACHY_GL_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "boost/asio.hpp"

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

#include <machycore.h>
#include <machyapi.h>
#include <machycontrol.h>

#define MASS_NO_MOMENT 0
#define NO_SIM 1

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
        GLfloat u_rot;
        GLfloat u_fps;
        GLfloat u_time;
        GLuint u_frame;
        GLfloat u_pos[2];
        GLfloat u_resolution[2];
        /* locations of uniforms in the proram */
        GLuint rot_location;
        GLuint time_location;
        GLuint pos_location;
        GLuint resolution_location;
        GLuint fps_location;
        GLuint frame_location;
        /* shaders data path */
        char *vertex_path, *fragment_path;
    };

    class Window
    {
        public:
            GLFWwindow* window;
#ifndef RESIZABLE
            int width;
            int height;
#endif
            Window(std::string start_width, std::string start_height)
            {
                if (!glfwInit())
                    exit(EXIT_FAILURE);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

                std::string::size_type sz;

                width = std::stoi(start_width, &sz);
                height = std::stoi(start_height, &sz);
                window = glfwCreateWindow(width, height, "MachyTech", NULL, NULL);

                if (!window)
                {
                    glfwTerminate();
                    exit(EXIT_FAILURE);
                }

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#ifndef RESIZABLE
                glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
#endif
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

    class scene
    {
        private:
            std::string vertex_shader_text;
            std::string fragment_shader_text;

            int vertex_flag, fragment_flag;

            const char* fs_text;
            const char* vs_text;

            long first_frame_time;

            int simulator_scenario;
            std::vector<std::string> info;
            machygl_variables *machygl_var;
            machygl::Window* win_;
            machycore::controller_data* controller_;
            steady_timer frameticker_;
            steady_timer shader_timer_;
            machycontrol::mass* simulation_mass;
        public:
            scene(Window& win, boost::asio::io_context& io_context, machyapi::client& c)
                : win_(&win),
                    frameticker_(io_context),
                    shader_timer_(io_context),
                    controller_(c._controller),
                    simulator_scenario(MASS_NO_MOMENT),
                    machygl_var(new machygl::machygl_variables())
            {}
            scene(Window&win, boost::asio::io_context& io_context)
                : win_(&win),
                    frameticker_(io_context),
                    shader_timer_(io_context),
                    simulator_scenario(NO_SIM),
                    machygl_var(new machygl::machygl_variables())
            {}
            GLuint vertex_shader, fragment_shader;
            void render();
            void set_image_shader(std::string);
            void realize();
            void realize_shader();
            void start(std::string);
            void tick();
            std::string read_shader(std::string direction);
            int get_compile_data(GLuint shader);
            GLuint link_shader_old(std::string vs_direction, std::string fs_direction);
            bool link_shader(std::string ver_src, std::string frag_src);
            void change_shader_timer_1();
            void change_shader_timer_2();
    };
}
#endif