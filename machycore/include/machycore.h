#ifndef MACHYCORE_H
#define MACHYCORE_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <mutex>
#include <iostream>
#include <atomic>
#include "envs.h"

namespace machycore
{   
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

    long current_time_ms();

    struct controller_data{
        // joystick data
        float normalizedAngle;
        float normalizedMagnitude;
        // connection
        std::atomic<bool> connected;
        // mutex
        std::mutex mtx_;
    };
}
#endif