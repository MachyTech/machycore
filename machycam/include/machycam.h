#ifndef MACHYCAM_H_
#define MACHYCAM_H_

#include <iostream>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <mutex>
#include <opencv4/opencv2/opencv.hpp>
#include "machycore.h"
#include "machyvision.h"
#include <chrono>

#define RTSP_STREAM 0
#define READ_IMAGE 1
#define USB_CAM 2

namespace machycam
{
    class cam_session
    {
        public:
            cam_session(boost::asio::io_context& io_context, 
                        boost::asio::thread_pool& pool,
                        machycore::texture_data* texture,
                        machycore::camera_data* cam, int type)
                :io_context_(io_context),
                pool_(pool),
                ticker_(io_context),
                texture_(texture),
                type_(type),
                cam_(cam)
            {
                switch(type)
                {
                    case RTSP_STREAM:
                        printf("starting rtsp stream...\n");
                        boost::asio::post(pool_, [this](){for(;;){start_rtsp();};});
                        break;
                    case READ_IMAGE:
                        boost::asio::post(io_context, [this](){read_image();});
                        break;
                    case USB_CAM:
                        boost::asio::post(pool_, [this](){for(;;){start_usb();}});
                        break;
                }
            }
        private:
            boost::asio::io_context& io_context_;
            boost::asio::thread_pool& pool_;
            machycore::texture_data* texture_;
            machycore::camera_data* cam_;
            boost::asio::steady_timer ticker_;
            int type_;

            void start_rtsp();
            void read_image();
            void start_usb();
    };
}

#endif