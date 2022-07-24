#include "machypose.h"

namespace machypose
{
    void ORBSLAM::detect_features()
    {
        cv::Mat local_frame;
        cv::Mat grey_frame;
        while(true)
        {
            int64 t = cv::getTickCount();

            cam_->mtx_.lock();
            cam_->frame.copyTo(local_frame);
            cam_->mtx_.unlock();
            
            cv::cvtColor(local_frame, grey_frame, cv::COLOR_BGR2GREY);
            cv::imshow("display", grey_frames);
            cv::waitKey(25);

            printf(" FPS %f", 3);

            texture_->mtx_.lock();
            texture_->height = local_frame.rows;
            texture_->width = local_frame.cols;
            texture_->image = local_frame.data;
            if(texture_->dirty)
                {
                    texture_->dirty = false;
                };
            texture_->mtx_.unlock();  

            t = cv::getTickCount() - t;
        }
    }
}
