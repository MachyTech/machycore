#ifndef TRAJECTORY_SCENE_H
#define TRAJECTORY_SCENE_H

#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <string>

#include <machycore.h>
#include "machygl.h"
#include <api.h>

namespace machyscene
{
    /*
     * machyscenes
     */
    class Scene
    {
        protected:
            GLuint program, n_points;
        public:
            Scene(GLuint a)
            {
                program = a;
            }
            virtual void bind_buffer() {};
            virtual void print_buffer() {};
            virtual void render(GLFWwindow* win) {};
            virtual void render(GLFWwindow* win, int linewidth, int samplesize) {};
            virtual ~Scene() { };
    };

    class TrajectorySim : public Scene
    {
        private:
            GLuint vertex_array_object, buffer, rot_location, vpos_location, off_location, len_location; 
        public:
            TrajectorySim ( GLuint a ) : Scene(a)
            {
                std::cout<<"starting trajectory sim"<<std::endl;
                glCreateVertexArrays(1, &vertex_array_object);
                glBindVertexArray(vertex_array_object);
                std::cout<<"succesfully binding vertex"<<std::endl;
                rot_location = glGetUniformLocation(program, "ROT");
                off_location = glGetUniformLocation(program, "OFF");
                vpos_location = glGetAttribLocation(program, "position");
                len_location = glGetUniformLocation(program, "LEN");

                glGenBuffers(1, &buffer);
                glBindBuffer(GL_ARRAY_BUFFER, buffer);
                //machycore::print_vpos_data();
                //print_buffer();
                print_buffer();
                bind_buffer();
            }
            void bind_buffer();
            void print_buffer();
            void render(GLFWwindow* win, int linewidth, int samplesize);
            ~TrajectorySim()
            {
                glDeleteVertexArrays(1, &vertex_array_object);
                glDeleteProgram(program);
            }
    };

    class PlotData : public Scene
    {
        private:
            GLuint vertex_array_object, buffer, vpos_location, len_location, off_location;
        public:
            PlotData ( GLuint a ) : Scene(a)
            {
                glCreateVertexArrays(1, &vertex_array_object);
                glBindVertexArray(vertex_array_object);

                vpos_location = glGetAttribLocation(program, "position");
                len_location = glGetUniformLocation(program, "LEN");
                off_location = glGetUniformLocation(program, "OFF");

                glGenBuffers(1, &buffer);
                glBindBuffer(GL_ARRAY_BUFFER, buffer);
                print_buffer();
                n_points = machycore::trajectory->size();
                bind_buffer();
                //machycore::trajectory->clear();
                //print_buffer();
            }
            void bind_buffer();
            void print_buffer();
            void render(GLFWwindow* win);
            ~PlotData()
            {
                glDeleteVertexArrays(1, &vertex_array_object);
                glDeleteProgram(program);
            }
    };

/*     class Triangle : public Scene
    {
        private: 
            GLuint vertex_array_object, buffer;
        public:
            Triangle ( GLuint a ) : Scene(a)
            {
                glCreateVertexArrays(1, &vertex_array_object);
                glBindVertexArray(vertex_array_object);

                glGenBuffers(1, &buffer);
                glBindBuffer(GL_ARRAY_BUFFER, buffer);
            }
            void bind_buffer();
            void render(GLFWwindow* win);
            ~Triangle()
            {
                glDeleteVertexArrays(1, &vertex_array_object);
                glDeleteProgram(program);
            }
    }; */
}

#endif