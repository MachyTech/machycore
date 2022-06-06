#ifndef _MACHY_GL_H_
#define _MACHY_GL_H_

#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

namespace machygl
{
    class utils
    {
        private:
            std::string vertex_shader_text;
            std::string fragment_shader_text;

            int vertex_flag, fragment_flag;

            const char* fs_text;
            const char* vs_text;

            std::vector<std::string> info;
        public:
            GLuint vertex_shader, fragment_shader;
            std::string read_shader(std::string direction);
            int get_compile_data(GLuint shader);
            GLuint link_shader(std::string vs_direction, std::string fs_direction);
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
}
#endif