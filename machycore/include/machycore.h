#ifndef MACHYCORE_H
#define MACHYCORE_H

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <vector>

#include <mutex>
#include <thread>
#include <condition_variable>

#include <iostream>
#include "envs.h"

namespace machycore
{
    /*
     * machycore includes function that connect all the pieces
     * of software together
     */
    struct Data{
        float x, y;
        Data(float a[2]) : x(a[0]/10), y(a[1]/10)
        {} 
    };
    /*  
     * data structure with x and y data accepted by the openGL context
     */
    extern std::vector<Data> *trajectory;

    struct Sim{
        float x, y, t, v, theta;
        Sim(float a[5]) : x(a[0]/10), y(a[1]/10), t(a[2]), v(a[3]), theta(a[4]+1.57)
        {}
    };
    /*
     * data structure with x and y data. Also it uses simulation
     * data available from the path trajectory.
     */
    extern std::vector<Sim> *virposition;
    
    // thread synchronization
    extern std::mutex m_machydata;
    extern std::condition_variable render_ready;
    extern bool load_scene;
    extern bool scene_loaded;

    void print_pos_data();
    /*
     * print data structures from machycore
     */
    void print_vpos_data();
    /*
     * overload function print virtual data structure
     * from machytech core
     */
    void print_data_stats();
    
    class Environment
    {
        std::vector<Variables*> *variables;
        public:
            Environment(){
                variables = new std::vector<Variables*>;
            }
            ~Environment(){
                std::vector<Variables*>::iterator it;

                for (it = variables->begin(); it < variables->end(); it++)
                {
                    delete *it;
                }
                delete variables;
            }

            void appendVariable(Variables* variable)
            {
                variables->push_back( variable );
            }
            
            std::string get(int varNum)
            {
                std::vector<Variables*>::iterator it = variables->begin () + varNum;
                return (*it)->get_var();
            }

            void print()
            {
                std::vector<Variables*>::iterator it;
                for (it = variables->begin (); it < variables->end (); it++)
                {
                    std::string str = (*it)->get_var();
                    (*it)->print();
                }
            }
    };

    struct controller_data{
        float normalizedLX;
        float normalizedLY;
        float normalizedMagnitude;
        int xSuccess;
    };
}
#endif