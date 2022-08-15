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

#include <Eigen/Dense>

#define MAX_NO_WINDOWS 2

#define MASS_NO_MOMENT 0
#define NO_SIM 1
#define RAW_CAMERA_OUTPUT 2
#define HOG_DESCRIPTOR_OUTPUT 3

enum scene_enum {
    ROTATING_TRIANGLE = 0,
    RAW_CAMERA = 1
};

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

    struct uniform{
        /* MVP matrix */
        Eigen::Matrix3f mvp;
        /* uniform values */
        GLfloat u_fps;
        GLfloat u_time;
        GLfloat u_frame;
        /* locations of uniforms in the program */
        GLuint time_location;
        GLuint frame_location;
        GLuint fps_location;
        GLuint mvp_location;
    };
    
    struct Vertex
    {
        float x, y, z;
        float r, g, b;
        float u, v;
        float nx, ny, nz;
    };
    
    class shader
    {
        private:
            GLuint program;

            std::string image_shader;
            std::string vertex_shader_text;
            std::string fragment_shader_text;

            machygl::uniform * uniforms;
            
            bool shader_dirty;
            void realize();
            void update_uniforms()
            {
                if(uniforms->time_location != -1)
                    glUniform1f (uniforms->time_location, uniforms->u_time);
                if (uniforms->fps_location != -1)
                    glUniform1f (uniforms->fps_location, uniforms->u_fps);
                if (uniforms->frame_location != -1)
                    glUniform1i (uniforms->frame_location, uniforms->u_frame);
            }
            std::string read_shader(std::string direction);
            int get_compile_data(GLuint shader);
            bool link_shader(std::string ver_src, std::string frag_src);
        public:
            shader(std::string direction)
                : uniforms(new uniform)
            {
                this->set_image_shader(direction);
                realize();
            }
            void use()
            {
                if(this->shader_dirty)
                    realize();
                glUseProgram(this->program);
            }

            void unuse()
            {
                glUseProgram(0);
            }
            void set_image_shader(std::string direction);

    };

    class scene
    {
        private:
            int scenario;

            int first_frame_time;

            std::vector<shader*> shaders;

            bool scene_dirty;

            machygl_variables *machygl_var;
            GLFWwindow* win_;
            machycore::controller_data* controller_;
            machycore::texture_data* texture_;
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
                    machygl_var(new machygl::machygl_variables())
            {}
            scene(GLFWwindow* win, boost::asio::thread_pool& pool, boost::asio::io_context& io_context, machycore::texture_data* texture, int scene)
                : win_(win),
                    io_context_(io_context),
                    pool_(pool),
                    texture_(texture),
                    scenario(scene),
                    machygl_var(new machygl::machygl_variables())
            {
                boost::asio::post(pool_, [this](){start();});
            }
            void render();
            void realize_scene();
            void realize();
            void start();
            void tick();
            void link_textures();
    };
}
#endif