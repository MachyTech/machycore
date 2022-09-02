#ifndef _MACHYPOSE_H_
#define _MACHYPOSE_H_

#include <opencv4/opencv2/opencv.hpp>
#include <iomanip>
#include <mutex>
#include <vector>
#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/io_context.hpp>

#include "machycore.h"

#include "ORB_SLAM3/System.h"

#define MONOCULAR 0
#define STEREO 1
namespace machypose
{
    unsigned char* cvMat2TexInput(cv::Mat& img);
    class ORBSLAM 
    {
        public:
            ORBSLAM(boost::asio::io_context& io_context,
                boost::asio::thread_pool& pool,
                machycore::pose_data* pose_data,
                machycore::feature_data* feature_data,
                machycore::yolo_data* yolo_data,
                machycore::pose_object_data* pose_object_data,
                machycore::texture_data* texture,
                machycore::camera_data* cam,
                machycore::camera_data* cam2,
                int type):
            pool_(pool),
            io_context_(io_context),
            pose_data_(pose_data),
            feature_data_(feature_data),
            yolo_data_(yolo_data),
            pose_object_data_(pose_object_data),
            texture_(texture),
            cam_(cam),
            cam2_(cam2),
            type_(type)
            {
                switch(type)
                {
                    case MONOCULAR:
                        boost::asio::post(pool_, [this]() {detectMoncocular();});
                        break;
                    case STEREO:
                        boost::asio::post(pool_, [this]() {detectStereo();});
                        break;
                }
            };
        private:
            boost::asio::io_context& io_context_;
            boost::asio::thread_pool& pool_;
            machycore::pose_data* pose_data_;
            machycore::feature_data* feature_data_;
            machycore::yolo_data* yolo_data_;
            machycore::pose_object_data* pose_object_data_;
            machycore::texture_data* texture_;
            machycore::camera_data* cam_;
            machycore::camera_data* cam2_;
            int type_;
            void detectMonocular();
            void detectStereo();
    };
}
#endif