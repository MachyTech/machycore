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

#define MAX_NO_WINDOWS 2

#define MASS_NO_MOMENT 0
#define NO_SIM 1
#define RAW_CAMERA_OUTPUT 2
#define HOG_DESCRIPTOR_OUTPUT 3

using boost::asio::steady_timer;

namespace machygl
{
    struct machygl_variables{
        std::string image_shader;
        bool image_shader_dirty;
        bool error_set;
        /* variable to hold our handle to the vertex array object */
        GLuint vao;
        GLuint vao_2;
        /* buffer objects */
        GLuint buffer[3];
        GLuint element_buffer;
        /* active program */
        GLuint program;
        /* textures */
        unsigned int tex;
        /* uniform values */
        GLfloat u_rot;
        GLfloat u_fps;
        GLfloat u_time;
        GLuint u_frame;
        GLfloat u_pos[2];
        GLfloat u_resolution[2];
        GLfloat u_tex0resolution[2];
        /* locations of uniforms in the proram */
        GLuint rot_location;
        GLuint time_location;
        GLuint pos_location;
        GLuint resolution_location;
        GLuint fps_location;
        GLuint frame_location;
        GLuint tex0_location;
        GLuint tex0res_location;
        /* shaders data path */
        char *vertex_path, *fragment_path;
    };

    class Window
    {
        public:
            GLFWwindow* window[MAX_NO_WINDOWS];
            int window_count;
            Window(std::string start_width, std::string start_height)
            {
                if (!glfwInit())
                    exit(EXIT_FAILURE);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

                std::string::size_type sz;

                int width = std::stoi(start_width, &sz);
                int height = std::stoi(start_height, &sz);
                window[0] = glfwCreateWindow(width, height, "MachyTech", NULL, NULL);

                if (!window[0])
                {
                    glfwTerminate();
                    exit(EXIT_FAILURE);
                }

                window_count = 1;
            }
            Window(int start_width, int start_height, std::string name)
            {
                if (!glfwInit())
                    exit(EXIT_FAILURE);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

                window[0] = glfwCreateWindow(start_width, start_height, name.c_str(), NULL, NULL);

                if (!window[0])
                {
                    printf("failed to create window!\n");
                    glfwTerminate();
                    exit(EXIT_FAILURE);
                }

                window_count = 1;
            }
            ~Window(void)
            {
                glfwDestroyWindow(window[0]);
                glfwTerminate();
                exit(EXIT_SUCCESS);
            }
            void add_window(int start_width, int start_height, std::string name)
            {
                window_count++;
                if(window_count>MAX_NO_WINDOWS)
                {   
                    printf("Failed to create window! Max number of windows reached...\n"); 
                    return;
                }

                if (!glfwInit())
                    exit(EXIT_FAILURE);
                
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

                window[window_count-1] = glfwCreateWindow(start_width, start_height, name.c_str(), NULL, window[0]);
                
                if (!window[window_count - 1])
                {
                    glfwTerminate();
                    exit(EXIT_FAILURE);
                }
            }
    };

    class bounding_box
    {
        public:
            GLfloat x, y, width, height;
            rectangle(int x, int y, int width, int height, int window_width, int window_height) :
                x((GLfloat) window_width/(x-window_width/2)),
                y((GLfloat) window_height / (y-window_height/2)),
                width((GLfloat) window_width / (width-window_width/2),
                height((GLfloat) window_height / (height-window_height/2))
            {
                const GLfloat vertex_data[] = {
                    x, y, 0.0f,
                    x, y+height, 0.0f,
                    x+width,  y+height, 0.0f,
                    x, y, 0.0f,
                };

                const GLfloat colorData[] = {
                    1.0f, -1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                };

                const GLfloat texture_data[] = {
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    0.0f, 0.0f,
                };

                realize_shader();
                machygl_var->image_shader_dirty = false;

                glGenVertexArrays (1, vao);
                glBindVertexArray(vao);

                glGenBuffers(3, &buffer[0]);
                glGenBuffers(1, &element_buffer);

                /* vertex data */
                glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(0);
                
                /* color data */
                glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(1);
                
                /* texture data */
                glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(texture_data), texture_data, GL_STATIC_DRAW);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(2);

                GLuint indices[] = {
                    0, 1, 2,
                    3, 4, 5
                };

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
            }
        private:
            GLuint vao;
            GLuint element_buffer;
            GLuint buffer[3];
    }

    class scene
    {
        private:
            std::string vertex_shader_text;
            std::string fragment_shader_text;

            int vertex_flag, fragment_flag;

            const char* fs_text;
            const char* vs_text;

            long first_frame_time;

            int scenario;

            std::vector<std::string> info;
            machygl_variables *machygl_var;
            GLFWwindow* win_;
            machycore::controller_data* controller_;
            machycore::texture_data* texture_;
            boost::asio::steady_timer timer_1_;
            boost::asio::steady_timer timer_2_;
            machycontrol::mass* simulation_mass;
            boost::asio::io_context& io_context_;
            boost::asio::thread_pool& pool_;
        public:
            scene(GLFWwindow* win, boost::asio::thread_pool& pool, boost::asio::io_context& io_context, machyapi::client& c)
                : win_(win),
                    io_context_(io_context),
                    pool_(pool),
                    controller_(c._controller),
                    scenario(MASS_NO_MOMENT),
                    timer_1_(io_context),
                    timer_2_(io_context),
                    machygl_var(new machygl::machygl_variables())
            {}
            scene(GLFWwindow* win, boost::asio::thread_pool& pool, boost::asio::io_context& io_context, machycore::texture_data* texture, int scene)
                : win_(win),
                    io_context_(io_context),
                    pool_(pool),
                    texture_(texture),
                    scenario(scene),
                    timer_1_(io_context),
                    timer_2_(io_context),
                    machygl_var(new machygl::machygl_variables())
            {
                boost::asio::post(pool_, [this](){start("shaders/image.glsl");});
            }
            
            GLuint vertex_shader, fragment_shader;

            void render();
            void set_image_shader(std::string);
            void realize();
            void realize_2();
            void realize_shader();
            void start(std::string);
            void tick();
            std::string read_shader(std::string direction);
            int get_compile_data(GLuint shader);
            bool link_shader(std::string ver_src, std::string frag_src);
            void link_textures();
    };
}
#endif