#ifndef _MACHYVISION_H_
#define _MACHYVISION_H_

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

#include "yolo_v2_class.hpp"

#include "machycore.h"

namespace machyvision
{
    unsigned char* cvMat2TexInput(cv::Mat& img);
    class hogDetector
    {
        enum Mode { Default, Daimler } m;
        cv::HOGDescriptor hog, hog_d;
        public:
            hogDetector(boost::asio::io_context& io_context,
                    boost::asio::thread_pool& pool,
                    machycore::texture_data* texture,
                    machycore::camera_data* cam) 
                : m(Default), pool_(pool), cam_(cam), texture_(texture), io_context_(io_context), hog(), 
                hog_d(cv::Size(48, 96), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9)
            {
                hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
                hog_d.setSVMDetector(cv::HOGDescriptor::getDaimlerPeopleDetector());
                boost::asio::post(pool_, [this]() {for(;;){detect();}});
            }
            void toggleMode() { m = (m == Default ? Daimler : Default); }
            std::string modeName() const { return (m == Default ? "Default" : "Daimler"); }
            std::vector<cv::Rect> hog_result(cv::InputArray img)
            {
                std::vector<cv::Rect> found;
                if (m == Default)
                    hog.detectMultiScale(img, found, 0, cv::Size(8,8), cv::Size(), 1.05, 2, false);
                else if (m == Daimler)
                    hog_d.detectMultiScale(img, found, 0, cv::Size(8,8), cv::Size(), 1.05, 2, true);
                return found;
            }
            void adjustRect(cv::Rect & r) const
            {
                r.x += cvRound(r.width*0.1);
                r.width = cvRound(r.width*0.8);
                r.y += cvRound(r.height*0.07);
                r.height = cvRound(r.height*0.8);
            }
            void detect();
        private:
            boost::asio::io_context& io_context_;
            boost::asio::thread_pool& pool_;
            machycore::camera_data* cam_;
            machycore::texture_data* texture_;
    };

    class YOLO
    {
        public:
            YOLO(boost::asio::io_context& io_context,
                boost::asio::thread_pool& pool,
                machycore::texture_data* texture,
                machycore::yolo_data* objs,
                machycore::camera_data* cam):
            pool_(pool),
            io_context_(io_context),
            texture_(texture),
            objs_(objs),
            cam_(cam)
            {
                boost::asio::post(pool_, [this]() {detect();});
            } 
            void detect();
        private:
            boost::asio::io_context& io_context_;
            boost::asio::thread_pool& pool_;
            machycore::camera_data* cam_;
            machycore::texture_data* texture_;
            machycore::yolo_data* objs_;
    };
}
#endif