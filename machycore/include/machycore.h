#ifndef MACHYCORE_H
#define MACHYCORE_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <mutex>
#include <iostream>
#include <atomic>
#include "envs.h"
#include <opencv4/opencv2/opencv.hpp>

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

    struct yolo_obj {
        /* start coordinates */
        int x, y;
        /* wdith and height */
        int width, height;
        /* id : 0 is person */
        int id;
        /* probability */
        float prob;
        yolo_obj(int x,int y,int width,int height,int id,float prob) : x(x),
         y(y), width(width), height(height), id(id), prob(prob)
         {}
    };

    struct yolo_data {
        /* dynamic array with yolo objects */
        std::vector<yolo_obj> obj_array;    
        /* mutex */
        std::mutex mtx_;
    };

    struct controller_data{
        // joystick data
        float normalizedAngle;
        float normalizedMagnitude;
        // connection
        std::atomic<bool> connected;
        // mutex
        std::mutex mtx_;
    };

    struct texture_data{
        /* width and height of texture in pixels */
        int width, height;
        /* image in bytes */
        unsigned char* image;
        /* clean */
        int dirty;
        /* mutex for thread safe usage */
        std::mutex mtx_;
        /* constructor */
        texture_data() : dirty(true){ printf("texture got copied!\n"); }
    };

    struct camera_data{
        /* opencv frame for convenience in other algorithms */
        cv::Mat frame;
        int width, height, type;
        /* fps of video capture */
        int64 fps;
        /* is the camera connected */
        std::atomic<bool> connected;
        /* mutex for threadsafe usage */
        std::mutex mtx_;
    };
}
#endif