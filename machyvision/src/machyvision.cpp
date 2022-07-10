#include "machyvision.h"

namespace machyvision
{
    unsigned char* cvMat2TexInput(cv::Mat& img)
    {
        cv::Mat img_out_1;
        cv::cvtColor(img, img_out_1, cv::COLOR_BGR2RGB);
        cv::Mat img_out_2;
        cv::flip(img_out_1, img_out_2, 0);
        return img_out_2.data;
    }


    void Detector::detect()
    {
        toggleMode();
        toggleMode();
        for(;;)
        {
            int64 t = cv::getTickCount();
            /* copy frame locally to save time in critical section */
            cv::Mat local_frame;
            cv::Mat preview_frame;
            cam_->mtx_.lock();
            cam_->frame.copyTo(local_frame);
            cam_->frame.copyTo(preview_frame);
            cam_->mtx_.unlock();
            
            std::vector<cv::Rect> found = hog_result(local_frame);  

            for (std::vector<cv::Rect>::iterator i = found.begin(); i != found.end(); ++i)
            {
                cv::Rect &r = *i;
                Detector::adjustRect(r);
                rectangle(local_frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
            }

            cv::flip(preview_frame, preview_frame, 0);
            cv::cvtColor(preview_frame, preview_frame, cv::COLOR_BGR2RGB);

            texture_->mtx_.lock()   ;
            texture_->height = preview_frame.rows;
            texture_->width = preview_frame.cols;
            texture_->image = local_frame.data;
            if(texture_->dirty){texture_->dirty = false;};
            texture_->mtx_.unlock();

            t = cv::getTickCount() - t;

            std::cout<<"Mode : "<<modeName();
            printf(" FPS: %f dimensions {%d}{%d}\n", cv::getTickFrequency() / (double) t, texture_->height, texture_->width);
        }
    }
}