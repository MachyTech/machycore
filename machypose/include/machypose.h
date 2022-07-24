#ifndef _MACHYPOSE_H_
#define _MACHYPOSE_H_

#include <opencv4/opencv2/opencv.hpp>
#include <iomanip>
#include <mutex>
#include <iostream>
#include <vector>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/io_context.hpp>

#include "machycore.h"

namespace machypose
{
    unsigned char* cvMat2TexInput(cv::Mat& img);
    class ORBSLAM 
    {
        public:
            ORBSLAM(boost::asio::io_context& io_context,
                boost::asio::thread_pool& pool,
                machycore::texture_data* texture,
                machycore::camera_data* cam):
            pool_(pool),
            io_context_(io_context),
            texture_(texture),
            cam_(cam)
            {
                boost::asio::post(pool_, [this]() {detect();});
            } 
            void detect_features();
        private:
            boost::asio::io_context& io_context_;
            boost::asio::thread_pool& pool_;
            machycore::camera_data* cam_;
            machycore::texture_data* texture_;
    };
    class SVO
    {
        public:
        private:
    };
}
#endif