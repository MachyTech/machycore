#ifndef MACHYCAM_H_
#define MACHYCAM_H_

#include <iostream>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <mutex>
#include <cstdint>

#include <opencv4/opencv2/opencv.hpp>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "machycore.h"
#include "machyvision.h"
#include <chrono>

#define RTSP_STREAM 0
#define READ_IMAGE 1
#define USB_CAM 2
#define SCREEN_CAPTURE 3

namespace machycam
{
    class screenshot
    {
        Display* display;
        Window root;
        int x, y, width, height;
        XImage* img{nullptr};
    public:
        screenshot(int x, int y, int width, int height): 
            x(x),
            y(y),
            width(width),
            height(height)
        {
            display = XOpenDisplay(nullptr);
            root = DefaultRootWindow(display);
        }

        void run (cv::Mat& cvImg)
        {
            if(img != nullptr)
                XDestroyImage(img);
            img = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);
            cvImg = cv::Mat(height, width, CV_8UC4, img->data);
        }

        ~screenshot()
        {
            if(img != nullptr)
                XDestroyImage(img);
            XCloseDisplay(display);
        }
    };
    
    class cam_session
    {
        public:
            cam_session(boost::asio::io_context& io_context, 
                        boost::asio::thread_pool& pool,
                        machycore::texture_data* texture,
                        machycore::camera_data* cam, int type)
                : io_context_(io_context),
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
                        boost::asio::post(pool_, [this](){for(;;){start_usb();};});
                        break;
                    case SCREEN_CAPTURE:
                        boost::asio::post(pool_, [this](){start_screencapture_new();});
                        break;
                }
            };
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
            void capture_x11image(std::vector<uint8_t>& Pixels, int& width, int& height, int& BitsPerPixel);
            void start_screencapture();
            void start_screencapture_new();
    };
}

#endif