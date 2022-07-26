#include "machypose.h"

namespace machypose
{
    void ORBSLAM::detect_features()
    {
        cv::Mat local_frame;
        cv::Mat gray_frame;
        while(true)
        {
            int64 t = cv::getTickCount();

            cam_->mtx_.lock();
            cam_->frame.copyTo(local_frame);
            cam_->mtx_.unlock();

            if(local_frame.data)
            {
                cv::cvtColor(local_frame, local_frame, cv::COLOR_RGB2GRAY);
                cv::cvtColor(local_frame, local_frame, cv::COLOR_RGB2BGR);
                //cv::flip(local_frame, local_frame, 0);
            }

            //printf(" FPS %f", 3);

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
