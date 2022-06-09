#include "machygl.h"

namespace machygl
{
    std::string machygraph_vertex_shader = 
        "#version 430\n"
        "\n"
        "layout (location = 0) in vec3 VertexPosition;\n"
        "layout (location = 1) in vec3 VertexColor;\n"
        "layout (location = 2) in vec2 VertexTexture;\n"
        "\n"
        "out vec3 Color;\n"
        "out vec2 Texture;\n"
        ""
        "\n"
        "\n"
        "void main()\n"
        "{\n"
        "   Color = VertexColor;\n"
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
        "uniform mat4 mvp;\n"
        "uniform vec2 resolution;\n"
        "uniform vec4 InnerColor;\n"
        "uniform vec4 OuterColor;\n"
        "float RadiusInner;\n"
        "float RadiusOuter;\n";
    
    std::string machygraph_frag_suffix =
        "\nvoid main() {\n"
        "   vec4 c;\n"    
        "   machygraph_main(c, Color, Texture);\n"
        "   FragColor = c;\n"
        "}\n";

    enum {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,

        N_AXIS
    };

    /* rotation angles on each axis */
    static float rotation_angles[N_AXIS] = { 0.0 };

    std::string utils::read_shader(std::string direction)
    {
        std::ifstream in(direction);
        std::string contents((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
        std::string shader_text = contents.c_str();
        return shader_text;
    }

    void utils::tick ()
    {
        float previous_time;

        frame++;
        
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        long frame_time = value.count();

        if (machygl_var->first_frame_time == 0)
        {
            machygl_var->first_frame_time = frame_time;
            machygl_var->first_frame = frame;
            previous_time = 0;
        }
        else
            previous_time = machygl_var->time;
        
        machygl_var->time = (frame_time - machygl_var->first_frame_time) /  1000000.0f;
        machygl_var->frame = frame - machygl_var->first_frame;
        machygl_var->timedelta = machygl_var->time - previous_time;
        render();
        frameticker_.expires_after(std::chrono::milliseconds(1000));
        frameticker_.async_wait(std::bind(&utils::tick, this));
    }

    void utils::render()
    {
        int width, height;
        glfwGetFramebufferSize(win_, &width, &height);

        if (machygl_var->image_shader_dirty)
            realize_shader();
        
        /* clear the viewport */
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        compute_mvp (machygl_var->mvp,
                rotation_angles[X_AXIS],
                rotation_angles[Y_AXIS],
                rotation_angles[Z_AXIS]);

        GLfloat outerColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        GLfloat innerColor[] = {1.0f, 1.0f, 0.75f, 1.0f};

        glUseProgram(machygl_var->program);

        /* update uniforms */
        if(machygl_var->mvp_location != -1) 
            glUniformMatrix4fv (machygl_var->mvp_location, 1, GL_FALSE, &machygl_var->mvp[0]);  
        if(machygl_var->InnerColor_location != -1)
            glUniform4fv (machygl_var->InnerColor_location, 1, outerColor);
        if(machygl_var->OuterColor_location != -1)
            glUniform4fv (machygl_var->OuterColor_location, 1, innerColor);
        if(machygl_var->RadiusInner_location != -1)
            glUniform1f (machygl_var->RadiusInner_location, machygl_var->RadiusInner);
        if(machygl_var->RadiusOuter_location != -1)
            glUniform1f (machygl_var->RadiusOuter_location, machygl_var->RadiusOuter);
        if(machygl_var->resolution_location != -1)
            glUniform2fv (machygl_var->resolution_location, 1, machygl_var->resolution);

        /* use our vertices */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[1]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[2]);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindVertexArray(machygl_var->vao);
        glDrawArrays(GL_TRIANGLES, 0, 6 );

        /* we finished using the buffers and program */
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

        glFlush();

        glfwSwapBuffers(win_);
        glfwPollEvents();
        if (glfwWindowShouldClose(win_)){
            exit(1);
        }
    }

    void utils::set_image_shader(std::string shader)
    {
        machygl_var->image_shader.erase();
        machygl_var->image_shader = shader;

        if (machygl_var->error_set)
        {
            machygl_var->error_set = false;
        }
        machygl_var->image_shader_dirty = true;
    }

    void utils::realize()
    {    
        /* Draw two triangles across whole screen (no-locality of reference in cache) */
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
        
        const GLfloat textureData[] = {
            1.0f, -1.0f, 
            0.0f, 1.0f, 
            0.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, -1.0f,
            0.0f, 1.0f,
        };

        glGenVertexArrays (1, &machygl_var->vao);
        glBindVertexArray(machygl_var->vao);

        glGenBuffers(3, &machygl_var->buffer[0]);

        /* vertex data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        /* color data */
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);
        /* texture data */
        //printf("binding texture data to buffer\n");
        glBindBuffer(GL_ARRAY_BUFFER, machygl_var->buffer[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(textureData), textureData, GL_STATIC_DRAW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        realize_shader();
    }

    void utils::start(std::string vs_start_dir)
    {
        std::string shader = read_shader(vs_start_dir);
        set_image_shader(shader);
        realize();
        frameticker_.async_wait(std::bind(&utils::tick, this));
    }

    void utils::realize_shader()
    {
        std::string fragment_shader = machygraph_frag_prefix + machygl_var->image_shader + machygraph_frag_suffix;

        if (!link_shader(machygraph_vertex_shader, fragment_shader))
        {
            machygl_var->error_set = true;
        }
        fragment_shader.erase();

        /* start the new shader at time zero */
        machygl_var->first_frame_time = 0;
        machygl_var->first_frame = 0;

        machygl_var->RadiusInner = 0.25f;
        machygl_var->RadiusOuter = 0.45f;

        machygl_var->image_shader_dirty = false;
    }

    bool utils::link_shader(std::string ver_src, std::string frag_src)
    {
        bool res = true;

        const char *vs_src = ver_src.c_str();
        const char *fs_src = frag_src.c_str();
        printf("vs : %s\n", vs_src);
        printf("fs : %s\n", fs_src);

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vs_src, NULL);
        glCompileShader(vertex_shader);
        if (vertex_flag==1){
            std::cout<<"Error when compiling"<<std::endl;
            glDeleteShader (vertex_shader);
            return false;
        }

        delete vs_src;

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fs_src, NULL);
        glCompileShader(fragment_shader);

        fragment_flag = get_compile_data(fragment_shader);
        if (fragment_flag==1){
            std::cout<<"Error when compile the fragment shader"<<std::endl;
            glDeleteShader (fragment_shader);
            return false;
        }

        delete fs_src;

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
        machygl_var->mvp_location = glGetUniformLocation (program, "mvp");
        machygl_var->InnerColor_location = glGetUniformLocation (program, "InnerColor");
        machygl_var->OuterColor_location = glGetUniformLocation (program, "OuterColor");
        machygl_var->RadiusInner_location = glGetUniformLocation (program, "RadiusInner");
        machygl_var->RadiusOuter_location = glGetUniformLocation (program, "RadiusOuter");
        machygl_var->resolution_location = glGetUniformLocation (program, "Resolution");
        
        glDetachShader (program, vertex_shader);
        glDetachShader (program, fragment_shader);
        glDeleteShader (vertex_shader);
        glDeleteShader (fragment_shader);
        return res;
    }

    GLuint utils::link_shader_old(std::string vs_direction, std::string fs_direction)
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

    int utils::get_compile_data(GLuint shader)
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
    void 
    utils::compute_mvp (float *res,
                float  phi,
                float  theta,
                float  psi)
    {
        float x = phi * (M_PI / 180.f);
        float y = theta * (M_PI / 180.f);
        float z = psi * (M_PI / 180.f);
        float c1 = cosf (x), s1 = sinf (x);
        float c2 = cosf (y), s2 = sinf (y);
        float c3 = cosf (z), s3 = sinf (z);
        float c3c2 = c3 * c2;
        float s3c1 = s3 * c1;
        float c3s2s1 = c3 * s2 * s1;
        float s3s1 = s3 * s1;
        float c3s2c1 = c3 * s2 * c1;
        float s3c2 = s3 * c2;
        float c3c1 = c3 * c1;
        float s3s2s1 = s3 * s2 * s1;
        float c3s1 = c3 * s1;
        float s3s2c1 = s3 * s2 * c1;
        float c2s1 = c2 * s1;
        float c2c1 = c2 * c1;

        /* initialize to the identity matrix */
        res[0] = 1.f; res[4] = 0.f;  res[8] = 0.f; res[12] = 0.f;
        res[1] = 0.f; res[5] = 1.f;  res[9] = 0.f; res[13] = 0.f;
        res[2] = 0.f; res[6] = 0.f; res[10] = 1.f; res[14] = 0.f;
        res[3] = 0.f; res[7] = 0.f; res[11] = 0.f; res[15] = 1.f;

        /* apply all three rotations using the three matrices:
        *
        * ⎡  c3 s3 0 ⎤ ⎡ c2  0 -s2 ⎤ ⎡ 1   0  0 ⎤
        * ⎢ -s3 c3 0 ⎥ ⎢  0  1   0 ⎥ ⎢ 0  c1 s1 ⎥
        * ⎣   0  0 1 ⎦ ⎣ s2  0  c2 ⎦ ⎣ 0 -s1 c1 ⎦
        */
        res[0] = c3c2;  res[4] = s3c1 + c3s2s1;  res[8] = s3s1 - c3s2c1; res[12] = 0.f;
        res[1] = -s3c2; res[5] = c3c1 - s3s2s1;  res[9] = c3s1 + s3s2c1; res[13] = 0.f;
        res[2] = s2;    res[6] = -c2s1;         res[10] = c2c1;          res[14] = 0.f;
        res[3] = 0.f;   res[7] = 0.f;           res[11] = 0.f;           res[15] = 1.f;
    }

}
