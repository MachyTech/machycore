#include "machygl.h"

#define SHOW_FPS

namespace machygl
{
    std::string machygraph_vertex_shader = 
        "#version 460\n"
        "\n"
        "layout (location = 0) in vec3 VertexPosition;\n"
        "layout (location = 1) in vec3 VertexColor;\n"
        "\n"
        "out vec3 Color;\n"
        "out vec2 Texture;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   Color = VertexColor;\n"
        "   // use only x and y data\n"
        "   Texture = VertexPosition.xy;\n"
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
        "uniform vec2 u_resolution;\n"
        "uniform float u_rot;\n"
        "uniform vec3 u_pos;\n"
        "uniform float u_time;\n"
        "uniform float u_fps;\n"
        "uniform int u_frame;\n";
    
    std::string machygraph_frag_suffix =
        "\nvoid main() {\n"
        "   vec4 c;\n"    
        "   machygraph_main(c, Color, Texture);\n"
        "   FragColor = c;\n"
        "}\n";

    std::string scene::read_shader(std::string direction)
    {
        std::ifstream in(direction);
        std::string contents((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
        std::string shader_text = contents.c_str();
        return shader_text;
    }

    void scene::tick ()
    {        
        float previous_time = machygl_var->u_time;

        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        long current_frame_time = value.count();

        machygl_var->u_time = (current_frame_time - first_frame_time) / 1000.0;
        machygl_var->u_fps = 1 / (machygl_var->u_time - previous_time);        
        
        if(machygl_var->u_frame==0)
            first_frame_time = current_frame_time;

        machygl_var->u_frame++;


#ifdef SHOW_FPS
        printf("elapsed time: %f, elapsed frames: %d, fps: %f\n", machygl_var->u_time, machygl_var->u_frame, machygl_var->u_fps);
#endif
        render();
        frameticker_.expires_after(std::chrono::milliseconds(1));
        frameticker_.async_wait(std::bind(&scene::tick, this));
    }

    void scene::render()
    {
#ifdef RESIZABLE
        int width, height;
        glfwGetFramebufferSize(win_, &width, &height);
#endif
        if (machygl_var->image_shader_dirty)
            realize_shader();
        
        /* clear the viewport */
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(machygl_var->program);
        if(controller_->connected)
            if (controller_->mtx_.try_lock())
            {
                printf("{normalizedLX: %f, normalizedLY: %f, normalizedMagnitude: %f}\n", 
                    controller_->normalizedLX, controller_->normalizedLY, controller_->normalizedMagnitude);      
                controller_->mtx_.unlock();
            }

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

        /* use our vertices */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[1]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindVertexArray(machygl_var->vao);
        glDrawArrays(GL_TRIANGLES, 0, 6 );

        /* we finished using the buffers and program */
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

        glFlush();

        glfwSwapBuffers(win_->window);
        glfwPollEvents();
        if (glfwWindowShouldClose(win_->window)){
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
        
        realize_shader();

        glGenVertexArrays (1, &machygl_var->vao);
        glBindVertexArray(machygl_var->vao);

        glGenBuffers(3, &machygl_var->buffer[0]);

        /* vertex data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        /* color data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void scene::start(std::string vs_start_dir)
    {
        machygl_var->u_resolution[0] = win_->width;
        machygl_var->u_resolution[1] = win_->height;

        std::string shader = read_shader(vs_start_dir);
        set_image_shader(shader);
        realize();
        frameticker_.async_wait(std::bind(&scene::tick, this));
        change_shader_timer_1();
    }

    void scene::change_shader_timer_1()
    {
        std::string shader = read_shader("shaders/complex-field.glsl");
        set_image_shader(shader);
        shader_timer_.expires_after(std::chrono::milliseconds(1000));
        shader_timer_.async_wait(std::bind(&scene::change_shader_timer_2, this));
    }

    void scene::change_shader_timer_2()
    {
        std::string shader = read_shader("shaders/rectangle.glsl");
        set_image_shader(shader);
        shader_timer_.expires_after(std::chrono::milliseconds(1000));
        shader_timer_.async_wait(std::bind(&scene::change_shader_timer_1, this));
    }

    void scene::realize_shader()
    {
        std::string fragment_shader = machygraph_frag_prefix + machygl_var->image_shader + machygraph_frag_suffix;

        if (!link_shader(machygraph_vertex_shader, fragment_shader))
        {
            machygl_var->error_set = true;
        }
        fragment_shader.erase();

        /* start the new shader at time zero */
        machygl_var->u_frame = 0;

        machygl_var->image_shader_dirty = false;
    }

    bool scene::link_shader(std::string ver_src, std::string frag_src)
    {
        bool res = true;

        const char *vs_src = ver_src.c_str();
        const char *fs_src = frag_src.c_str();
        printf("vs : %s\n", vs_src);
        printf("fs : %s\n", fs_src);

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

        glDetachShader (program, vertex_shader);
        glDetachShader (program, fragment_shader);
        glDeleteShader (vertex_shader);
        glDeleteShader (fragment_shader);
        return res;
    }

    GLuint scene::link_shader_old(std::string vs_direction, std::string fs_direction)
    {
        vertex_shader_text = read_shader(vs_direction);
        vs_text = vertex_shader_text.c_str();
        std::cout<<"using vertex shader : \n"<<vs_text<<std::endl;
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vs_text, NULL);
        glCompileShader(vertex_shader);
        vertex_flag = get_compile_data(vertex_shader);
        if (vertex_flag==1){
            std::cout<<"Error when compiling"<<std::endl;
            throw;
        }
        std::cout<<"using vertex shader : \n"<<vs_text<<std::endl;

        fragment_shader_text = read_shader(fs_direction);
        fs_text = fragment_shader_text.c_str();

        std::cout<<"using the fragment shader : \n"<<fs_text<<std::endl;

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fs_text, NULL);
        glCompileShader(fragment_shader);

        fragment_flag = get_compile_data(fragment_shader);
        if (fragment_flag==1){
            std::cout<<"Error when compile the fragment shader"<<std::endl;
            throw;
        }

        std::cout<<"using the fragment shader : \n"<<fs_text<<std::endl;

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        return program;
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
}
