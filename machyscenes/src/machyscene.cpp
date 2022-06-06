#include "machyscene.h"

namespace machyscene
{
    void TrajectorySim::bind_buffer()
    {
        std::cout<<"-------------------------------position-buffer---------------------------\n";
        std::cout<<"array size : "<<machycore::virposition->size()<<std::endl;
        std::cout<<"array size in bytes : "<<machycore::virposition->size()*sizeof(machycore::virposition)<<std::endl;
        std::cout<<"pointer to first object : "<<machycore::virposition->data()<<std::endl;
        std::cout<<"byte size of array entry : "<<sizeof(machycore::virposition)<<std::endl;
        std::cout<<"byte size of first object : "<<sizeof(machycore::virposition->at(0))<<std::endl;
        std::cout<<"-------------------------------trajectory-buffer---------------------------\n";
        std::cout<<"array size : "<<machycore::trajectory->size()<<std::endl;
        std::cout<<"array size in bytes : "<<machycore::trajectory->size()*sizeof(machycore::trajectory)<<std::endl;
        std::cout<<"pointer to first object : "<<machycore::trajectory->data()<<std::endl;
        std::cout<<"byte size of array entry : "<<sizeof(machycore::trajectory)<<std::endl;
        std::cout<<"byte size of first object : "<<sizeof(machycore::trajectory->at(0))<<std::endl;
        std::cout<<"--------------------------------------------------------------------------\n";
        
        glBufferData(GL_ARRAY_BUFFER, machycore::trajectory->size()*sizeof(machycore::trajectory), machycore::trajectory->data(), GL_STATIC_DRAW);
        //glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, (*machycore::trajectory).size()*sizeof((*machycore::trajectory)), &(*machycore::trajectory)[0]->x);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(machycore::trajectory), NULL);
        glEnableVertexAttribArray(vpos_location);
    }

    void TrajectorySim::render(GLFWwindow* win, int linewidth, int samplesize)
    {
        double_t t_last = 0.0;
        clock_t ct_last = clock();
        for (int i=0; i<machycore::virposition->size()-1; i++)
        {
            double_t t_begin = machycore::virposition->at(i).t;
            clock_t ct_begin = clock();

            const GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glClearBufferfv(GL_COLOR, 0, color);
            
            int width, height;
            glfwGetFramebufferSize(win, &width, &height);
            glViewport(0, 0, width, height);
            glUseProgram(program);

            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::rotate(trans, -(float)machycore::virposition->at(i).theta, 
                glm::vec3(0.0f, 0.0f, 1.0f));

            glm::vec2 offset = glm::vec2(machycore::virposition->at(i).x, 
                        machycore::virposition->at(i).y);

            glUniform2fv(off_location, 1, glm::value_ptr(offset));
            glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(trans));
            glUniform1f(len_location, (GLfloat) samplesize/10);
            glDrawArrays(GL_LINE_STRIP, 0 , machycore::trajectory->size());
            glEnable(GL_LINE_SMOOTH);
            glLineWidth(linewidth);

            double t_end = machycore::virposition->at(i+1).t;
            clock_t ct_end = clock();
            double cdt = double(ct_end-ct_begin)/double(CLOCKS_PER_SEC);
            double dt = double(t_end - t_begin);

            printf(":\rfps : %lf", 1/cdt);

            if(double(dt-cdt)>0){usleep(double(dt-cdt)*1000000);}
            
            glfwSwapBuffers(win);
            glfwPollEvents();
            if (glfwWindowShouldClose(win)){
                exit(1);
            }
        }
    }
    void TrajectorySim::print_buffer()
    {
        std::cout<<"printing buffer... t, v, theta\n";
        for (int i=0; i<machycore::virposition->size(); i++){
            std::cout<<i<<" "<<&machycore::virposition->at(i).x<<", "<<machycore::virposition->at(i).x
            <<", "<<&machycore::virposition->at(i).y<<", "<<machycore::virposition->at(i).y
            <<", "<<&machycore::virposition->at(i).t<<", "<<machycore::virposition->at(i).t
            <<", "<<&machycore::virposition->at(i).v<<", "<<machycore::virposition->at(i).v
            <<", "<<&machycore::virposition->at(i).theta<<", "<<machycore::virposition->at(i).theta<<std::endl;
        }
    }

    void PlotData::print_buffer()
    {
        std::cout<<"printing buffer.... x, y\n";
        for (int i=0; i<machycore::trajectory->size(); i++)
        {
            std::cout<<i<<" "<<&machycore::trajectory->at(i).x<<", "<<machycore::trajectory->at(i).x<<", "<<&machycore::trajectory->at(i).y<<", "<<machycore::trajectory->at(i).y<<std::endl;
        }
    }

    void PlotData::bind_buffer()
    {
        std::cout<<"-------------------------------buffer---------------------------\n";
        std::cout<<"array size : "<<machycore::trajectory->size()<<std::endl;
        std::cout<<"array size in bytes : "<<machycore::trajectory->size()*sizeof(machycore::trajectory)<<std::endl;
        std::cout<<"pointer to first object : "<<machycore::trajectory->data()<<std::endl;
        std::cout<<"byte size of array entry : "<<sizeof(machycore::trajectory)<<std::endl;
        std::cout<<"byte size of first object : "<<sizeof(machycore::trajectory->at(0))<<std::endl;
        std::cout<<"-----------------------------------------------------------------\n";
        glBufferData(GL_ARRAY_BUFFER, machycore::trajectory->size()*sizeof(machycore::trajectory), machycore::trajectory->data(), GL_STATIC_DRAW);
        //glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, (*machycore::trajectory).size()*sizeof((*machycore::trajectory)), &(*machycore::trajectory)[0]->x);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(machycore::trajectory), NULL);
        glEnableVertexAttribArray(vpos_location);
    }

    void PlotData::render(GLFWwindow* win)
    {
        const GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, color);

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glUseProgram(program);
        
        glm::vec2 offset = glm::vec2(0.0 , 0.0);
        glUniform2fv(off_location, 1, glm::value_ptr(offset));
        glUniform1f(len_location, (GLfloat) 50/10);
        glDrawArrays(GL_LINE_STRIP, 0, n_points);
        glLineWidth(5.0);
        //glDrawArrays(GL_POINTS,0 ,n_points);
        glEnable(GL_LINE_SMOOTH);
        //glPointSize(5.0);

        glfwSwapBuffers(win);
        glfwPollEvents();
        if (glfwWindowShouldClose(win)){
            exit(1);
        }
    }
}