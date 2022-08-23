#include "machygl.h"

namespace machygl
{

    std::string machygraph_vertex_shader = 
        "#version 460\n"
        "\n"
        "layout (location = 0) in vec3 VertexPosition;\n"
        "layout (location = 1) in vec3 VertexColor;\n"
        "layout (location = 2) in vec2 VertexTexture;\n"
        "layout (location = 3) in vec3 VertexNormal;\n"
        "\n"
        "uniform mat4 mvp;\n"
        "\n"
        "out vec3 vs_position;\n"
        "out vec3 vs_color;\n"
        "out vec2 vs_texcoord;\n"
        "out vec3 vs_normal;\n"
        "\n"
        "mat4 aMat4 = mat4(1.0, 0.0, 0.0, 0.0,\n"
        "          0.0, 1.0, 0.0, 0.0,\n"
        "          0.0, 0.0, 1.0, 0.0,\n"  // 3. column
        "          0.0, 0.0, 0.0, 1.0);\n"
        "void main()\n"
        "{\n"
        "   vs_position = vec4(mvp * vec4(VertexPosition, 1.f)).xyz;\n"
        "   vs_color = VertexColor;\n"
        "   // use only x and y data\n"
        "   vs_texcoord = vec2(VertexTexture.x, VertexTexture.y * -1.f);\n"
        "   gl_Position = mvp * vec4(VertexPosition, 1.0);\n"
        "}\n";
    
    std::string machygraph_frag_prefix =
        "#version 460\n"
        "\n"
        "in vec3 vs_position;\n"
        "in vec3 vs_color;\n"
        "in vec2 vs_texcoord;\n"
        "in vec3 vs_normal;\n"
        "\n"
        "\n"
        "layout (location = 0) out vec4 FragColor;\n"
        "\n"
        "uniform mat4 mvp;\n"
        "uniform sampler2D u_tex0;\n"
        "uniform float u_time;\n"
        "uniform float u_fps;\n"
        "uniform int u_frame;\n";
    
    std::string machygraph_frag_suffix =
        "\nvoid main() {\n"
        "   vec4 c;\n"    
        "   machygraph_main(c, vs_color, vs_texcoord);\n"
        "   FragColor = c;\n"
        "}\n";

    void scene::realize_scene()
    {
        switch(scenario)
        {            
            case ROTATING_TRIANGLE:
            {
                /* shaders */
                this->shaders.push_back(new shader("shaders/basic.glsl"));  
                /* meshes */
                std::vector<mesh*> meshes;
                std::vector<mesh*> meshes2;
                /* textures */
                //this->textures.push_back(new static_texture("media/lena.jpg", GL_TEXTURE_2D));
                this->textures.push_back(new dynamic_texture(texture_, GL_TEXTURE_2D));

                meshes.push_back(
                    new mesh(
                        new machygl::quad(),
                        Eigen::Vector3f{0.f,  0.f, 0.5f},
                        Eigen::Vector3f{0.0f, PI*1.f, PI*1.f},
                        Eigen::Vector3f{2.f, 2.f, 2.f}
                    )
                );

                //this->textures.push_back(new texture("media/beach.jpg", GL_TEXTURE_2D));
                this->models.push_back(new model(
                    Eigen::Vector3f{0.f, 0.f, 0.f},
                    shaders,
                    textures,
                    meshes
                ));
                
                meshes.clear();
                textures.clear();
                
                //for (auto i : this->textures)
                //    delete i;

                meshes.push_back(
                    new mesh(
                        new machygl::cube(),
                        Eigen::Vector3f{-0.3f,  0.f, 0.5f},
                        Eigen::Vector3f{0.25f*PI, 0.f, 0.f},
                        Eigen::Vector3f{0.2f, 0.2f, 0.2f}
                    )
                );

                this->textures.push_back(new static_texture("media/beach.jpg", GL_TEXTURE_2D));

                this->models.push_back(new model(
                    Eigen::Vector3f{0.f, 0.f, 0.f},
                    shaders,
                    textures,
                    meshes
                ));

                meshes.clear();
                textures.clear();
                //for (auto i : this->textures2)
                //    delete i;
                break;
            }
            case TEST_AR_SCENE:
            {
                /* shaders */
                this->shaders.push_back(new shader("shaders/basic.glsl"));
                /* meshes */
                std::vector<mesh*> meshes;
                std::vector<mesh*> meshes2;
                /* textures */
                //machygl::dynamic_texture* d_tex = new dynamic_texture(texture_, GL_TEXTURE_2D);
                //this->textures.push_back(new dynamic_texture(texture_, GL_TEXTURE_2D));
                this->textures.push_back(new static_texture("media/beach.jpg", GL_TEXTURE_2D));

                meshes.push_back(
                    new mesh(
                        new machygl::triangle(),
                        Eigen::Vector3f{0.0f,  0.f, 0.f},
                        Eigen::Vector3f{0.f, PI*1.f, PI*1.f},
                        Eigen::Vector3f{2.f, 2.f, 2.f}
                    )
                );

                this->models.push_back(new  model(
                    Eigen::Vector3f(0.f, 0.f, 0.f),
                    shaders,
                    textures,
                    meshes
                ));

                break;
            }
        }
        this->scene_dirty = false;
    }

    void scene::render()
    {   
        if(this->scene_dirty)
            realize_scene();
        
        /* clear the viewport */
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);  
        
        for (auto& i : this->models)
            i->render();
        
        //this->shaders[0]->printFPS();
        this->shaders[0]->tick();

        this->models[1]->rotateY(this->shaders[0]->getTime());
        //this->models[0]->rotateX(this->shaders[0]->getTime()*0.5);

        //glEnable(GL_BLEND);  
        
        glfwSwapBuffers(win_);
        glFlush();

        glBindVertexArray(0);
	    glUseProgram(0);
	    glActiveTexture(0);
	    glBindTexture(GL_TEXTURE_2D, 0);

        glfwSetFramebufferSizeCallback(win_, [](GLFWwindow* window, int width, int height){
            glViewport(0,0, width, height);
        });
        glfwPollEvents();
        if (glfwWindowShouldClose(win_)){
            exit(1);
        }
    }

    void mesh::initVAO()
    {    
        /* create the vao */
        glCreateVertexArrays(1, &this->VAO);
        glBindVertexArray(this->VAO);
        /* create the vbo */
        glGenBuffers(1, &this->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        
        /* bind buffer data */
        glBufferData(GL_ARRAY_BUFFER, this->nrOfVertices * sizeof(Vertex), this->vertexarray, GL_STATIC_DRAW);

        /* vertex data */
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, x));
        glEnableVertexAttribArray(0);
        
        /* color data */
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, r));
        glEnableVertexAttribArray(1);
        
        /* texture data */
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, u));
        glEnableVertexAttribArray(2);

        /* normal data */
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, nx));
        glEnableVertexAttribArray(3);
        
        /* generate EBO and Bind and send Data */
        if (this->nrOfIndices > 0)
        {
            glGenBuffers(1, &this->EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->nrOfIndices * sizeof(GLuint), this->indices, GL_STATIC_DRAW);
        }

        //glBindVertexArray(0);
    }

    void shader::set_image_shader(std::string direction)
    {
        this->image_shader.erase();
        this->image_shader = read_shader(direction);
        uniforms->u_frame = 0;

        this->shader_dirty = true;
    }

    std::string shader::read_shader(std::string direction)
    {
        std::ifstream in(direction);
        std::string contents((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
        std::string shader_text = contents.c_str();
        return shader_text;
    }

    int shader::get_compile_data(GLuint shader)
    {
        GLint isCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if(isCompiled == GL_FALSE)
        {
            /* ERROR handling */
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
            /* make sure we don't leak the shader */
            glDeleteShader(shader);
            for(int i=0; i<maxLength; i++){
                std::cout<<errorLog[i];
            }
            /* return 1 on error */
            return 1;
        }
        /* return 0 when succesfull */
        return 0;
    }

    void shader::realize()
    {
        std::string fragment_shader = machygraph_frag_prefix + this->image_shader + machygraph_frag_suffix;
        
        if (this->program !=0)
            glDeleteProgram(this->program);
        
        this->program = glCreateProgram();


        if (!link_shader(machygraph_vertex_shader, fragment_shader))
        {
            printf("error linking shader!\n");
        }
        fragment_shader.erase();

        this->shader_dirty = false;
    }

    bool shader::link_shader(std::string ver_src, std::string frag_src)
    {
        bool res = true;

        const char *vs_src = ver_src.c_str();
        const char *fs_src = frag_src.c_str();

        int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vs_src, NULL);
        glCompileShader(vertex_shader);
        int vertex_flag = get_compile_data(vertex_shader);

        if (vertex_flag==1){
            std::cout<<"Error when compiling"<<std::endl;
            glDeleteShader (vertex_shader);
            return false;
        }

        int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fs_src, NULL);
        glCompileShader(fragment_shader);

        int fragment_flag = get_compile_data(fragment_shader);
        if (fragment_flag==1){
            std::cout<<"Error when compile the fragment shader"<<std::endl;
            glDeleteShader (fragment_shader);
            return false;
        }

        glAttachShader(this->program, vertex_shader);
        glAttachShader(this->program, fragment_shader);
        glLinkProgram(this->program);

        int isLinked;

        glGetProgramiv (this->program, GL_LINK_STATUS, &isLinked);
        /* check if program is linked */
        if (isLinked == GL_FALSE)
        {
            GLint maxLength;
            glGetProgramiv (this->program, GL_INFO_LOG_LENGTH, &maxLength);
            GLchar *buffer = (char *) malloc (maxLength + 1);
            glGetProgramInfoLog (this->program, maxLength, NULL, buffer);

            std::cout << "Linking failure:\n" << buffer << std::endl;
            res = false;
            free(buffer);

            glDeleteProgram (this->program);
            
            glDeleteShader (vertex_shader);
            glDeleteShader (fragment_shader);
            return res;
        }
        uniforms->mvp_location = glGetUniformLocation(this->program, "mvp");
        uniforms->time_location = glGetUniformLocation (this->program, "u_time");
        uniforms->fps_location = glGetUniformLocation (this->program, "u_fps");
        uniforms->frame_location = glGetUniformLocation (this->program, "u_frame");

        glDetachShader (this->program, vertex_shader);
        glDetachShader (this->program, fragment_shader);
        glDeleteShader (vertex_shader);
        glDeleteShader (fragment_shader);
        return res;

    }

    void scene::start()
    {   
        int width, height;
        glfwGetFramebufferSize(win_, &width, &height);
        
        machygl_var->u_resolution[0] = width;
        machygl_var->u_resolution[1] = height;

        glfwSetInputMode(win_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        //glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
        glfwMakeContextCurrent(win_);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            exit(-1);
        }    
        glfwSwapInterval(1);
        
        realize_scene();

        for(;;){render();};
    }

    void scene::realize()
    {    
        glGenTextures(1, &machygl_var->tex);
        glBindTexture(GL_TEXTURE_2D, machygl_var->tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

/*
    void scene::tick ()
    {        
        float previous_time = machygl_var->u_time;

        long current_frame_time = machycore::current_time_ms();

        machygl_var->u_time = (current_frame_time - first_frame_time) / 1000.0;
        machygl_var->u_fps = 1 / (machygl_var->u_time - previous_time);        
        
        if(machygl_var->u_frame==0)
            first_frame_time = current_frame_time;

        machygl_var->u_frame++;

        //printf("elapsed time: %f, elapsed frames: %d, fps: %f\n", machygl_var->u_time, machygl_var->u_frame, machygl_var->u_fps);

        render();
    }
*/

    /* more c++ like implementation of shader linker -> worth investigation which is better */
    /*
    GLuint scene::link_shader_old(std::string vs_direction, std::string fs_direction)
    {
        vertex_shader_text = read_shader(vs_direction);
        vs_text = vertex_shader_text.c_str();
        switch(scenario)
        {
            case CAMERA:
                std::cout<<"[raw camera] using vertex shader : \n"<<vs_text<<std::endl;
            case DETECTOR_OUTPUT:
                std::cout<<"[detector output] using vertex shader : \n"<<vs_text<<std::endl;
            default:
                std::cout<<"using vertex shader : \n"<<vs_text<<std::endl;
        }
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vs_text, NULL);
        glCompileShader(vertex_shader);
        vertex_flag = get_compile_data(vertex_shader);
        if (vertex_flag==1){
            std::cout<<"Error when compiling"<<std::endl;
            throw;
        }

        fragment_shader_text = read_shader(fs_direction);
        fs_text = fragment_shader_text.c_str();

        switch(scenario)
        {
            case CAMERA:
                std::cout<<"[raw camera] using the fragment shader : \n"<<fs_text<<std::endl;
            case DETECTOR_OUTPUT:
                std::cout<<"[detector output] using fragment shader : \n"<<fs_text<<std::endl;
            default:
                std::cout<<"using fragment shader : \n"<<fs_text<<std::endl;
        }
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fs_text, NULL);
        glCompileShader(fragment_shader);

        fragment_flag = get_compile_data(fragment_shader);
        if (fragment_flag==1){
            std::cout<<"Error when compile the fragment shader"<<std::endl;
            throw;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        return program;
    }
    */
    
    /* example of how to change a shader */
    /*
    void scene::change_shader_on_timer()
    {
        std::string shader = read_shader("shaders/complex-field.glsl");
        set_image_shader(shader);
        shader_timer_.expires_after(std::chrono::milliseconds(1000));
        shader_timer_.async_wait(std::bind(&scene::change_shader_timer_2, this));
    }
    */
}
