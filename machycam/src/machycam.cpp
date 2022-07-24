#include "machycam.h"

namespace machycam{
    void cam_session::capture_x11image(std::vector<uint8_t>& Pixels, int& width, int& height, int& BitsPerPixel)
    {
        Display* display = XOpenDisplay(nullptr);
        Window root = DefaultRootWindow(display);
        
        XWindowAttributes attributes = {0};
        XGetWindowAttributes(display, root, &attributes);

        width = 400;
        height = 400;

        XImage* img = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
        BitsPerPixel = img->bits_per_pixel;
        Pixels.resize(width * height, 4);

        memcpy(&Pixels[0], img->data, Pixels.size());

        XDestroyImage(img);
        XCloseDisplay(display);
    }

    void cam_session::start_screencapture()
    {
        int width = 0;
        int height = 0;
        int Bpp = 0;
        std::vector<std::uint8_t> Pixels;
        
        Display* display = XOpenDisplay(nullptr);
        Window root = DefaultRootWindow(display);
    

        capture_x11image(Pixels, width, height, Bpp);

        if (width && height)
        {
            cam_->connected = 1;
            int64 t = cv::getTickCount ();

            cv::Mat img = cv::Mat(height, width, Bpp > 24 ? CV_8UC4 : CV_8UC3, &Pixels[0]);

            cam_->mtx_.lock();
            img.copyTo(cam_->frame);
            cam_->fps = cv::getTickCount()-t;
            cam_->mtx_.unlock();

            texture_->mtx_.lock();
            texture_->width = img.cols;
            texture_->height = img.rows;
            texture_->image = img.data;
            if(texture_->dirty){texture_->dirty = false;};
            texture_->mtx_.unlock();                
        }
    }

    void cam_session::start_screencapture_new()
    {
        int width = 800;
        int height = 600;
        int x = 0;
        int y = 0;
        machycam::screenshot screen(0, 0, width, height);
        cv::Mat img;

        while(1)
        {
            int64 t = cv::getTickCount ();

            screen.run(img);
        
            cam_->mtx_.lock();
            img.copyTo(cam_->frame);
            cv::cvtColor(img, cam_->frame, cv::COLOR_RGBA2RGB);
            cam_->fps = cv::getTickCount()-t;
            cam_->mtx_.unlock();

            texture_->mtx_.lock();
            texture_->width = 800;
            texture_->height = 600;
            texture_->image = machyvision::cvMat2TexInput(img);
            if(texture_->dirty=true){texture_->dirty = false;};
            texture_->mtx_.unlock();

            t = cv::getTickCount() - t;
        }
    }

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
            //std::cout << "Can not open video stream\n";
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
            if(texture_->mtx_.try_lock())
            {
                texture_->width = local_frame.cols;
                texture_->height = local_frame.rows;
                texture_->image = machyvision::cvMat2TexInput(local_frame);
                texture_->mtx_.unlock();
            }
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