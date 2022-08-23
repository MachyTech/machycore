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

#include "stb_image.h"

#include <Eigen/Dense>

#define PI 3.14

#define MAX_NO_WINDOWS 2

#define MASS_NO_MOMENT 0
#define NO_SIM 1
#define RAW_CAMERA_OUTPUT 2
#define HOG_DESCRIPTOR_OUTPUT 3

enum scene_enum {
    ROTATING_TRIANGLE = 0,
    RAW_CAMERA = 1,
    TEST_AR_SCENE = 2
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
        Eigen::Matrix4f mvp;
        /* uniform values */
        GLfloat u_fps;
        GLfloat u_time;
        GLint u_frame;
        /* locations of uniforms in the program */
        GLuint time_location;
        GLuint frame_location;
        GLuint fps_location;
        GLuint mvp_location;
        /* variable to store first time */
        long first_frame_time;
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
            
            long first_frame_time;
            float previous_time;

            bool shader_dirty;
            void realize();
            void update_uniforms()
            {
                if(uniforms->mvp_location != -1)
                    glUniformMatrix4fv(uniforms->mvp_location, 1, GL_FALSE, uniforms->mvp.data());
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
            GLfloat getTime(){
                return this->uniforms->u_time;
            };
            
            void tick()
            {
                float previous_time = uniforms->u_time;

                long current_frame_time = machycore::current_time_ms();

                uniforms->u_time = (current_frame_time - this->first_frame_time) / 1000.0;
                uniforms->u_fps = 1 / (uniforms->u_time - previous_time);

                if(uniforms->u_frame==0)
                    this->first_frame_time = current_frame_time;
                
                uniforms->u_frame++;
            }

            void use()
            {
                if(this->shader_dirty)
                    realize();
                glUseProgram(this->program);
                this->update_uniforms();
            }

            void printFPS()
            {
                printf("elapsed time: %f, elapsed frame: %d fps: %f\n", uniforms->u_time, uniforms->u_frame, uniforms->u_fps);
            }

            void unuse()
            {
                glUseProgram(0);
            }
            void set_image_shader(std::string direction);
            void setMVP(Eigen::Matrix4f MVP)
            {
                uniforms->mvp = MVP;
            }
    };
    class texture
    {
        private:
            GLuint id;
            unsigned int type;
        public:
            texture() {
                printf("did i get copied??\n");
            }
            virtual ~texture() {
                glDeleteTextures(1, &this->id);
            }
            
            GLuint getID() const { return this->id; }
            unsigned int getType() const { return this->type; }

            void setType(unsigned int type) { this->type = type; }
            void setID(GLuint id) { this->id = id; }
            
            virtual void bind(const GLint texture_unit) { };

            virtual void print() const { }

            virtual void unbind() {}

            virtual texture* clone() const { return (new texture(*this)); }
    };

    class dynamic_texture : public texture
    {
        private:
            machycore::texture_data* tex;
        public:
            dynamic_texture(machycore::texture_data* tex, GLenum type)
                : tex(tex), texture()
            {
                printf("[DYNAMIC] init\n");
                this->setType(type);
                GLuint id;
                glGenTextures(1, &id);
                this->setID(id);
                glBindTexture(this->getType(), this->getID());

                glTexParameteri(this->getType(), GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(this->getType(), GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(this->getType(), GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(this->getType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }

            virtual void bind(const GLint texture_unit)
            {
                
                this->tex->mtx_.lock();

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                    this->tex->width,
                    this->tex->height,
                    0, GL_RGB, GL_UNSIGNED_BYTE,
                    this->tex->image);
                
                this->tex->mtx_.unlock();
                glGenerateMipmap(this->getType());

                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(this->getType(), this->getID());
                
                this->unbind();

            }

            virtual void unbind() {
                glActiveTexture(0);
                glBindTexture(this->getType(), 0);
            }

            virtual dynamic_texture* clone() const { return(new dynamic_texture(*this)); };
    };

    class static_texture : public texture
    {
        private:
            int width, height;
        public:
            static_texture(const char* fileName, GLenum type)
            {
                printf("[STATIC] init\n");
                this->setType(type);
                int nrChannels;

                unsigned char *image = stbi_load(fileName, &this->width, &this->height, &nrChannels, 0);

                GLuint id;
                glGenTextures(1, &id);
                this->setID(id);

                glBindTexture(this->getType(), this->getID());
                
                glTexParameteri(this->getType(), GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(this->getType(), GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(this->getType(), GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(this->getType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                if (image)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
                    glGenerateMipmap(this->getType());
                }
                else
                {
                    std::cout << "ERROR::TEXTURE::TEXTURE_LOADING_FAILED: " << fileName <<"\n";
                }
                glActiveTexture(0);
                glBindTexture(this->getType(), 0);
                stbi_image_free(image);
            }

            virtual void bind(const GLint texture_unit)
            {
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(this->getType(), this->getID());
            }

            virtual void unbind() {
                glActiveTexture(0);
                glBindTexture(this->getType(), 0);
            }

            virtual static_texture* clone() const { return(new static_texture(*this)); };                
    };

    class primitive
    {
        private:
            std::vector<Vertex> vertices;
            std::vector<GLuint> indices;
        public:
            primitive() {}
            virtual ~primitive() {}

            void set(const Vertex* vertices, const unsigned nrOfVertices, 
                const GLuint* indices, const unsigned nrOfIndices)
            {
                for (size_t i = 0; i < nrOfVertices; i++)
                {
                    this->vertices.push_back(vertices[i]);
                }

                for (size_t i = 0; i < nrOfIndices; i++)
                {
                    this->indices.push_back(indices[i]);
                }
            }

            inline Vertex* getVertices() { return this->vertices.data(); }
            inline GLuint* getIndices() { return this->indices.data(); }
            inline const unsigned getNrOfVertices() { return this->vertices.size(); }
            inline const unsigned getNrOfIndices() { return this->indices.size(); }
    };

    class triangle : public primitive
    {
        public:
            triangle()
                : primitive()
            {
                Vertex vertices[] { 
                    //Position              //Color             //Texcoords     //Normals   
                    {0.f,   0.5f, 0.f,      1.f, 0.f, 0.f,      0.5f, 1.f,      0.f, 0.f, 1.f},
                    {-0.5f, -0.5f, 0.f,      0.f, 1.f, 0.f,      0.f, 0.f,       0.f, 0.f, 1.f},
                    {0.5f, -0.5f, 0.f,      0.f, 0.f, 1.f,      1.f, 0.f,       0.f, 0.f, 1.f}
                };

                unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

                GLuint indices[] =
                {
                    0, 1, 2
                };
                unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);
                this->set(vertices, nrOfVertices, indices, nrOfIndices);
            }
    };

    class quad : public primitive
    {
        public:
            quad()
                : primitive()
            {
                Vertex vertices[] { 
                    //Position              //Color             //Texcoords     //Normals   
        			{-0.5f, 0.5f, 0.f,	    1.f, 0.f, 0.f,		0.f, 1.f,		0.f, 0.f, 1.f},
        			{-0.5f, -0.5f, 0.f,		0.f, 1.f, 0.f,		0.f, 0.f,		0.f, 0.f, 1.f},
        			{0.5f, -0.5f, 0.f,		0.f, 0.f, 1.f,		1.f, 0.f,		0.f, 0.f, 1.f},
        			{0.5f, 0.5f, 0.f,	    1.f, 1.f, 0.f,		1.f, 1.f,		0.f, 0.f, 1.f}
               };

                unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

                GLuint indices[] =
                {
                    0, 1, 2,	//Triangle 1
                    0, 2, 3		//Triangle 2
                };
                unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);
                this->set(vertices, nrOfVertices, indices, nrOfIndices);
            }
    };

    class cube : public primitive
    {
        public:
            cube()
                : primitive()
            {
                /* based upon a net: there are only 8 position, but there are more than 8 distinct combinations */
                /* of position and texture coordinates */
                Vertex vertices[] { 
                    //Position                  //Color             //Texcoords     //Normals   
			        {-0.5f, -0.5f, -0.5f,       1.f, 0.f, 0.f,      0.f, 0.f,       0.f, 0.f, 1.f},
			        {0.5f, -0.5f, -0.5f,	    0.f, 1.f, 0.f,		1.f, 0.f,		0.f, 0.f, 1.f},
			        {0.5f, 0.5f, -0.5f,			0.f, 0.f, 1.f,		2.f, 0.f,		0.f, 0.f, 1.f},
			        {-0.5f, 0.5f, -0.5f,		1.f, 1.f, 0.f,		3.f, 0.f,		0.f, 0.f, 1.f},
                    {-0.5f, -0.5f, -0.5f,       1.f, 1.f, 1.f,      4.f, 0.f,       0.f, 0.f, 1.f},

			        {-0.5f, -0.5f, 0.5f,		1.f, 0.f, 0.f,		0.f, 1.f,		0.f, 0.f, -1.f},
			        {0.5f, -0.5f, 0.5f,	        0.f, 1.f, 0.f,		1.f, 1.f,		0.f, 0.f, -1.f},
			        {0.5f, 0.5f, 0.5f,		    0.f, 0.f, 1.f,		2.f, 1.f,		0.f, 0.f, -1.f},
			        {-0.5f, 0.5f, 0.5f,	        1.f, 1.f, 0.f,		3.f, 1.f,		0.f, 0.f, -1.f},
                    {-0.5f, -0.5f, 0.5f,        1.f, 1.f, 1.f,      4.f, 1.f,       0.f, 0.f, -1.f},
                    
                    {-0.5f, 0.5f, -0.5f,        1.f, 1.f, 1.f,      0.f, -1.f,       0.f, 0.f, -1.f},
                    {0.5f, 0.5f, -0.5f,          1.f, 1.f, 1.f,      1.f, -1.f,       0.f, 0.f, -1.f},

                    {-0.5, 0.5f, 0.5f,          1.f, 1.f, 1.f,      0.f, 2.f,       0.f, 0.f, -1.f},
                    {0.5f, 0.5f, 0.5f,          1.f, 1.f, 1.f,      1.f, 2.f,       0.f, 0.f, -1.f}
                };

                unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

                GLuint indices[] =
                {
                    0, 1, 5,  
                    5, 1, 6,
                    
                    1, 2, 6,  
                    6, 2, 7,
                    
                    2, 3, 7,  
                    7, 3, 8,
                    
                    3, 4, 8,  
                    8, 4, 9,
                    
                    10, 11, 0,  
                    0, 11, 1,
                    
                    5, 6, 12, 
                    12, 6, 13
                };

                unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);
                this->set(vertices, nrOfVertices, indices, nrOfIndices);
            }
    };

    class mesh
    {
        private:
            Vertex * vertexarray;
            GLuint * indices;

            unsigned int nrOfVertices;
            unsigned int nrOfIndices;

            GLuint VBO;
            GLuint VAO;
            GLuint EBO;

            Eigen::Vector3f position;
            Eigen::Vector3f origin;
            Eigen::Vector3f rotation;
            Eigen::Vector3f scale;
            
            Eigen::Matrix4f MVP;

            void initVAO();
            void updateMVP(machygl::shader* shader)
            {
                this->MVP = Eigen::Matrix4f::Identity();

                this->MVP.block<3,1>(0,3) += this->position;
                
                /* Euler Rotation */
                //printf("[MESH] euler rotations x=%f, y=%f, z=%f\n",this->rotation(0), this->rotation(1), this->rotation(2));
                Eigen::Matrix3f m;
                m = Eigen::AngleAxisf(this->rotation(0), Eigen::Vector3f::UnitX())
                    * Eigen::AngleAxisf(this->rotation(1), Eigen::Vector3f::UnitY())
                    * Eigen::AngleAxisf(this->rotation(2), Eigen::Vector3f::UnitZ());
                this->MVP.block<3,3>(0,0) = m;
                //Eigen::Affine3f position_transform(Eigen::Translation3f(this->position - this->origin));
                //this->MVP *= position_transform.matrix();
                Eigen::DiagonalMatrix<float, 3> m_d;
                m_d.diagonal() = this->scale;
            
                this->MVP.block<3,3>(0,0) *= m_d;

                shader->setMVP(this->MVP);
            }
        public:
            mesh(
                primitive* primitive,
                Eigen::Vector3f position,
                Eigen::Vector3f rotation,
                Eigen::Vector3f scale)
            {
                printf("[MESH] creating vertices\n");
                this->position = position;
                this->rotation = rotation;
                this->scale = scale;

                this->nrOfVertices = primitive->getNrOfVertices();
                this->nrOfIndices = primitive->getNrOfIndices();

                this->vertexarray = new Vertex[this->nrOfVertices];
                for (size_t i = 0; i < this->nrOfVertices; i++)
                {
                    this->vertexarray[i] = primitive->getVertices()[i];
                }

                this->indices = new GLuint[this->nrOfIndices];
                for (size_t i = 0; i < this->nrOfIndices; i++)
                {
                    this->indices[i] = primitive->getIndices()[i];
                }

                this->initVAO();
            }

            ~mesh()
            {
                /* some really weird compiler optimizations call the destructor when the pointers are deleted */
                printf("[MESH] cleaning up\n");
                //glDeleteVertexArrays(1, &this->VAO);
                //glDeleteBuffers(1, &this->VBO);

                //if (this->nrOfIndices > 0)
                //{
                //    glDeleteBuffers(1, &this->EBO);
                //}

                //delete[] this->vertexarray;
                //delete[] this->indices;
            }

            void render(machygl::shader* shader)
            {     
                this->updateMVP(shader);
                shader->use();           
                
                glBindVertexArray(this->VAO);
                
                glDrawElements(GL_TRIANGLES, this->nrOfIndices, GL_UNSIGNED_INT, 0);
        
                glBindVertexArray(0);
            }
            
            void move(const Eigen::Vector3f position)
            {
                this->position += position;
            }

            void rotateX(GLfloat x)
            {
                this->rotation(0) = x;
            }
            void rotateY(GLfloat y)
            {
                this->rotation(1) = y;
            }
    };

    class model
    {
        private:
            std::vector<mesh*> meshes;

            std::vector<shader*> shaders;

            std::vector<texture*> textures;

            Eigen::Vector3f position;
        public:
            model( 
                Eigen::Vector3f position,
                std::vector<shader*>& shaders,
                std::vector<machygl::texture*>& textures,
                std::vector<mesh*>& meshes
            )
            {
                this->position = position;

                for (auto* i : shaders)
                    this->shaders.push_back(new shader(*i));
                
                for (auto* i : textures)
                    this->textures.push_back(i->clone());

                for (auto* i : meshes)
                {
                    printf("[MODEL] adding meshes\n");
                    printf("%p\n", i);    
                    this->meshes.push_back(new mesh(*i));
                }

                for (auto& i : this->meshes)
                    i->move(this->position);
            }

            ~model()
            {
                for(auto*& i : this->meshes)
                    delete i;
            }

            void rotateX(GLfloat x)
            {
                for(auto& i : this->meshes)
                    i->rotateX(x);
            }

            void rotateY(GLfloat y)
            {
                for(auto& i : this->meshes)
                    i->rotateY(y);
            }

            void render()
            {
                this->textures[0]->bind(0);

                for (auto& i : this->meshes)
                {
                    i->render(this->shaders[0]);
                    this->textures[0]->unbind();
                }
                this->shaders[0]->unuse();
            }
    };

    class scene
    {
        private:
            int scenario;

            int first_frame_time;

            bool scene_dirty;

            GLuint VBO;
            GLuint VAO;
            GLuint EBO;

            std::vector<model*> models;
            
            std::vector<shader*> shaders;

            std::vector<texture*> textures;
            std::vector<texture*> textures2;


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