#include "machycam.h"

namespace machycam{
    /*
    void cam_session::show()
    {
        boost::asio::post(pool_,
        [this]()
        {
            for(;;)
            {
                if(cam_->connected!=0)
                {
                    std::ostringstream buf;
                    if(cam_->mtx_.try_lock())
                    { 
                        if (cam_->frame.empty())
                        {
                            std::cout << "Finished reading: empty frame\n";
                            cam_->connected = 0;
                            cam_->mtx_.unlock();
                            break;
                        }
                        buf << "FPS: " << std::fixed << std::setprecision(1) << (cv::getTickFrequency() / (double) cam_->fps);
                        cv::putText(cam_->frame, buf.str(), cv::Point(10, 30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255), 2, cv::LINE_AA);
                        cv::imshow("Video", cam_->frame);
                        cv::waitKey(1);
                    }
                    cam_->mtx_.unlock();
                }
            }
        });
    }
    */
    void cam_session::read_image()
    {
        cv::Mat img;
        printf("doing something\n");
        int width = 0;
        printf("doing something else \n");
        img = cv::imread("media/lenna.png");
        if ( !img.data )
        {
            std::cout<<"No image data \n";
            return;
        }
        texture_->mtx_.lock();
        texture_->width = img.cols;
        texture_->height = img.rows;
        texture_->image = machyvision::cvMat2TexInput(img);
        texture_->mtx_.unlock();
    }

    void cam_session::start_usb()
    {
        cv::VideoCapture cap;
        /* open usb camera with gstreamer pipeline */
        cap.open("v4l2src device=/dev/video0 ! videoconvert ! appsink", cv::CAP_GSTREAMER);
        if (!cap.isOpened())
        {
            cam_->connected=0;
            std::cout << "Can not open video stream\n";
            return;
        }
        cam_->connected = 1;
        for(;;)
        {
            int64 t = cv::getTickCount();
            /* store frame in current thread for less time in critical part */
            cv::Mat local_frame;
            cap >> local_frame;

            /* lock the camera struct and copy the local frame */
            cam_->mtx_.lock();
            cam_->frame = local_frame.clone();
            cam_->fps = cv::getTickCount() - t;
            cam_->mtx_.unlock();

            /* lock the texture struct and copy the image in bytes */
            texture_->mtx_.lock();
            texture_->width = local_frame.cols;
            texture_->height = local_frame.rows;
            texture_->image = machyvision::cvMat2TexInput(local_frame);
            texture_->mtx_.unlock();
        }
    }

    void cam_session::start_rtsp()
    {
        /* open the rtsp stream with opencv build-in rtsp client */
        cv::VideoCapture cap;
        cap.open("rtsp://0.0.0.0:8554/live");
        if (!cap.isOpened())
        {
            std::cout<< "Can not open video stream: \n";
            return;
        }
        printf("sucessfully opened rtsp stream\n");
        for(;;)
        {
            int64 t = cv::getTickCount ();
            /* store frame in local thread for less time in critical section */
            cv::Mat local_frame;
            cap >> local_frame;

            /* lock the cam struct and copy the local frame */
            cam_->mtx_.lock();
            local_frame.copyTo(cam_->frame);
            cam_->fps = cv::getTickCount() - t;
            cam_->mtx_.unlock();
            //printf("fps: %lf\n", cv::getTickFrequency() / (double) cam_->fps);
            /* lock the texture and copy the image in bytes */
            texture_->mtx_.lock();
            texture_->width = local_frame.cols;
            texture_->height = local_frame.rows;
            texture_->image = machyvision::cvMat2TexInput(local_frame);
            if(texture_->dirty){texture_->dirty = false;};
            texture_->mtx_.unlock();
        } 
    }
}