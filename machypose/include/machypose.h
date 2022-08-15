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

#include "System.h"

namespace machypose
{
    unsigned char* cvMat2TexInput(cv::Mat& img);
    class ORBSLAM 
    {
        public:
            ORBSLAM(boost::asio::io_context& io_context,
                boost::asio::thread_pool& pool,
                machycore::pose_data* pose_data,
                machycore::texture_data* texture,
                machycore::camera_data* cam):
            pool_(pool),
            io_context_(io_context),
            pose_data_(pose_data),
            texture_(texture),
            cam_(cam)
            {
                boost::asio::post(pool_, [this]() {detectMoncocular();});
            } 
            void detectMonocular();
        private:
            boost::asio::io_context& io_context_;
            boost::asio::thread_pool& pool_;
            machycore::pose_data* pose_data_;
            machycore::camera_data* cam_;
            machycore::texture_data* texture_;
    };
}
#endif