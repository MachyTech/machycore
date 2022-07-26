#include "machygl.h"

namespace machygl
{

    std::string machygraph_vertex_shader = 
        "#version 460\n"
        "\n"
        "layout (location = 0) in vec3 VertexPosition;\n"
        "layout (location = 1) in vec3 VertexColor;\n"
        "layout (location = 2) in vec2 VertexTexture;\n"
        "\n"
        "out vec3 Color;\n"
        "out vec2 Texture;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   Color = VertexColor;\n"
        "   // use only x and y data\n"
        "   Texture = VertexTexture;\n"
        "   gl_Position = vec4(VertexPosition, 1.0);\n"
        "}\n";
    
    std::string machygraph_frag_prefix =
        "#version 460\n"
        "\n"
        "in vec3 Color;\n"
        "in vec2 Texture;\n"
        "\n"
        "layout (location = 0) out vec4 FragColor;\n"
        "\n"
        "uniform vec2 u_tex0resolution;\n"
        "uniform sampler2D u_tex0;\n"
        "uniform vec2 u_resolution;\n"
        "uniform float u_rot;\n"
        "uniform vec2 u_pos;\n"
        "uniform float u_time;\n"
        "uniform float u_fps;\n"
        "uniform int u_frame;\n";
    
    std::string machygraph_frag_suffix =
        "\nvoid main() {\n"
        "   vec4 c;\n"    
        "   machygraph_main(c, Color, Texture);\n"
        "   FragColor = c;\n"
        "}\n";

    void scene::start(std::string vs_start_dir)
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
        
        std::string shader = read_shader(vs_start_dir);
        set_image_shader(shader);
        
        realize();

        for(;;){tick();};
    }

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

    void scene::render()
    {
        if (machygl_var->image_shader_dirty)
            realize_shader();
        
        /* clear the viewport */
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(machygl_var->program);
        
        switch (scenario)
        {
            case MASS_NO_MOMENT:
                if(controller_->connected)
                    if (controller_->mtx_.try_lock())
                        {
#ifdef DEBUG_CONTROLLER
                            printf("{normalizedAngle: %f, normalizedMagnitude: %f}\n", 
                            controller_->normalizedAngle, controller_->normalizedMagnitude);      
#endif    
                            machycontrol::mass_no_moment(
                            controller_->normalizedAngle, controller_->normalizedMagnitude, 
                            simulation_mass);
                            controller_->mtx_.unlock();
                        }
                    machygl_var->u_pos[0] = simulation_mass->s_x;
                    machygl_var->u_pos[1] = simulation_mass->s_y;
#ifdef DEBUG_MASSA        
                    printf("mass_x: %f, mass_y: %f\n", simulation_mass->s_x, simulation_mass->s_y);
#endif

                    break;
            case RAW_CAMERA_OUTPUT:
                texture_->mtx_.lock();
                machygl_var->u_tex0resolution[0] = texture_->width;
                machygl_var->u_tex0resolution[1] = texture_->height;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                    texture_->width, texture_->height, 
                    0, GL_RGB, GL_UNSIGNED_BYTE, texture_->image);
                texture_->mtx_.unlock();
                glGenerateMipmap(GL_TEXTURE_2D);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, machygl_var->tex);
                break;
        }
        glBindVertexArray(machygl_var->vao);
        machygl_var->u_rot = 1;

        /* update uniforms */
        if(machygl_var->rot_location != -1) 
            glUniform1f (machygl_var->rot_location, machygl_var->u_rot);
        if(machygl_var->time_location != -1)
            glUniform1f (machygl_var->time_location, machygl_var->u_time);
        if(machygl_var->pos_location != -1)
            glUniform2fv (machygl_var->pos_location, 1, machygl_var->u_pos);
        if(machygl_var->resolution_location != -1)
            glUniform2fv (machygl_var->resolution_location, 1, machygl_var->u_resolution);
        if (machygl_var->fps_location != -1)
            glUniform1f (machygl_var->fps_location, machygl_var->u_fps);
        if (machygl_var->frame_location != -1)
            glUniform1i (machygl_var->frame_location, machygl_var->u_frame);
        if (machygl_var->tex0_location != -1)
            glUniform1i (machygl_var->tex0_location, 0);
        if (machygl_var->tex0res_location != -1)
            glUniform2fv (machygl_var->tex0res_location, 1, machygl_var->u_tex0resolution);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, machygl_var->element_buffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(machygl_var->vao_2);

        /* update uniforms */
        machygl_var->u_rot = 0;
        if(machygl_var->rot_location != -1) 
            glUniform1f (machygl_var->rot_location, machygl_var->u_rot);
        if(machygl_var->time_location != -1)
            glUniform1f (machygl_var->time_location, machygl_var->u_time);
        if(machygl_var->pos_location != -1)
            glUniform2fv (machygl_var->pos_location, 1, machygl_var->u_pos);
        if(machygl_var->resolution_location != -1)
            glUniform2fv (machygl_var->resolution_location, 1, machygl_var->u_resolution);
        if (machygl_var->fps_location != -1)
            glUniform1f (machygl_var->fps_location, machygl_var->u_fps);
        if (machygl_var->frame_location != -1)
            glUniform1i (machygl_var->frame_location, machygl_var->u_frame);
        if (machygl_var->tex0_location != -1)
            glUniform1i (machygl_var->tex0_location, 0);
        if (machygl_var->tex0res_location != -1)
            glUniform2fv (machygl_var->tex0res_location, 1, machygl_var->u_tex0resolution);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, machygl_var->element_buffer);
        glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, 0);

        glEnable(GL_BLEND);  
        
        glUseProgram(0);
        glfwSwapBuffers(win_);
        glfwSetFramebufferSizeCallback(win_, [](GLFWwindow* window, int width, int height){
            glViewport(0,0, width, height);
        });
        glfwPollEvents();
        if (glfwWindowShouldClose(win_)){
            exit(1);
        }
    }

    void scene::set_image_shader(std::string shader)
    {
        machygl_var->image_shader.erase();
        machygl_var->image_shader = shader;
        if (machygl_var->error_set)
        {
            machygl_var->error_set = false;
        }
        machygl_var->image_shader_dirty = true;
    }

    void scene::realize()
    {    
        const GLfloat vertex_data[] = {
            -1.0f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,

            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
        };

        const GLfloat colorData[] = {
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            
            0.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,  
        };

        const GLfloat texture_data[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,

            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };
        
        realize_shader();
        machygl_var->image_shader_dirty = false;

        glGenVertexArrays (1, &machygl_var->vao);
        glBindVertexArray(machygl_var->vao);

        glGenBuffers(3, &machygl_var->buffer[0]);
        glGenBuffers(1, &machygl_var->element_buffer);

        /* vertex data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        
        /* color data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
        
        /* texture data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_data), texture_data, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);

        GLuint indices[] = {
            0, 1, 2,
            3, 4, 5
        };

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, machygl_var->element_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


        glGenTextures(1, &machygl_var->tex);
        glBindTexture(GL_TEXTURE_2D, machygl_var->tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void scene::realize_2()
    {
        const GLfloat vertex_data[] = {
            -0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f,
            0.5f,  0.5f, 0.0f,

            -0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
        };

        const GLfloat colorData[] = {
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            
            0.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,  
        };

        const GLfloat texture_data[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,

            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };
        
        glGenVertexArrays (1, &machygl_var->vao_2);
        glBindVertexArray(machygl_var->vao_2);

        glGenBuffers(3, &machygl_var->buffer[0]);
        glGenBuffers(1, &machygl_var->element_buffer);

        /* vertex data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        
        /* color data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
        
        /* texture data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_data), texture_data, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);

        GLuint indices[] = {
            0, 1, 2,
            3, 4, 5
        };

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, machygl_var->element_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


        glGenTextures(1, &machygl_var->tex);
        glBindTexture(GL_TEXTURE_2D, machygl_var->tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void scene::realize_shader()
    {
        std::string fragment_shader = machygraph_frag_prefix + machygl_var->image_shader + machygraph_frag_suffix;

        if (!link_shader(machygraph_vertex_shader, fragment_shader))
        {
            printf("error linking shader!\n");
            machygl_var->error_set = true;
        }
        fragment_shader.erase();
        /* start the new shader at time zero */
        machygl_var->u_frame = 0;
        
        /* create the physics object */
        // simulation_mass = new machycontrol::mass(1);

        machygl_var->image_shader_dirty = false;
    }

    bool scene::link_shader(std::string ver_src, std::string frag_src)
    {
        bool res = true;

        const char *vs_src = ver_src.c_str();
        const char *fs_src = frag_src.c_str();
        switch(scenario)
        {
            case RAW_CAMERA_OUTPUT:
                    printf("[RAW_CAMERA_OUTPUT] vs :\n %s\n", vs_src);
                    printf("[RAW_CAMERA_OUTPUT] fs :\n %s\n", fs_src);
                    break;
            default:
                    printf("vs :\n %s\n", vs_src);
                    printf("fs :\n %s\n", fs_src);
                    break;
        }

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vs_src, NULL);
        glCompileShader(vertex_shader);
        vertex_flag = get_compile_data(vertex_shader);

        if (vertex_flag==1){
            std::cout<<"Error when compiling"<<std::endl;
            glDeleteShader (vertex_shader);
            return false;
        }

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fs_src, NULL);
        glCompileShader(fragment_shader);

        fragment_flag = get_compile_data(fragment_shader);
        if (fragment_flag==1){
            std::cout<<"Error when compile the fragment shader"<<std::endl;
            glDeleteShader (fragment_shader);
            return false;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        int isLinked;

        glGetProgramiv (program, GL_LINK_STATUS, &isLinked);
        /* check if program is linked */
        if (isLinked == GL_FALSE)
        {
            GLint maxLength;
            glGetProgramiv (program, GL_INFO_LOG_LENGTH, &maxLength);
            GLchar *buffer = (char *) malloc (maxLength + 1);
            glGetProgramInfoLog (program, maxLength, NULL, buffer);

            std::cout << "Linking failure:\n" << buffer << std::endl;
            res = false;
            free(buffer);

            glDeleteProgram (program);
            
            glDeleteShader (vertex_shader);
            glDeleteShader (fragment_shader);
            return res;
        }
        if (machygl_var->program !=0)
            glDeleteProgram(machygl_var->program);

        machygl_var->program = program;
        machygl_var->rot_location = glGetUniformLocation (program, "u_rot");
        machygl_var->time_location = glGetUniformLocation (program, "u_time");
        machygl_var->pos_location = glGetUniformLocation (program, "u_pos");
        machygl_var->resolution_location = glGetUniformLocation (program, "u_resolution");
        machygl_var->fps_location = glGetUniformLocation (program, "u_fps");
        machygl_var->frame_location = glGetUniformLocation (program, "u_frame");
        machygl_var->tex0_location = glGetUniformLocation (program, "u_tex0");
        machygl_var->tex0res_location = glGetUniformLocation (program, "u_tex0resolution");

        glDetachShader (program, vertex_shader);
        glDetachShader (program, fragment_shader);
        glDeleteShader (vertex_shader);
        glDeleteShader (fragment_shader);
        return res;
    }

    int scene::get_compile_data(GLuint shader)
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

    std::string scene::read_shader(std::string direction)
    {
        std::ifstream in(direction);
        std::string contents((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
        std::string shader_text = contents.c_str();
        return shader_text;
    }

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
