#include "machyvision.h"

namespace machyvision
{
    unsigned char* cvMat2TexInput(cv::Mat& img)
    {
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        cv::flip(img, img, 0);
        return img.data;
    }


    void hogDetector::detect()
    {
        toggleMode();
        toggleMode();
        
        cv::Mat local_frame;

        for(;;)
        {
            int64 t = cv::getTickCount();
            /* copy frame locally to save time in critical section */
            cam_->mtx_.lock();
            cam_->frame.copyTo(local_frame);
            cam_->mtx_.unlock();
            
            std::vector<cv::Rect> found = hog_result(local_frame);  

            for (std::vector<cv::Rect>::iterator i = found.begin(); i != found.end(); ++i)
            {
                cv::Rect &r = *i;
                hogDetector::adjustRect(r);
                rectangle(local_frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
            }

            //cv::flip(local_frame, local_frame, 0);
            //cv::cvtColor(preview_frame, preview_frame, cv::COLOR_BGR2RGB);
            
            texture_->mtx_.lock();
            texture_->height = local_frame.rows;
            texture_->width = local_frame.cols;
            texture_->image = local_frame.data;
            if(texture_->dirty){texture_->dirty = false;};
            texture_->mtx_.unlock();

            t = cv::getTickCount() - t;

            std::cout<<"Mode : "<<modeName();
            printf(" FPS: %f dimensions {%d}{%d}\n", cv::getTickFrequency() / (double) t, texture_->height, texture_->width);
        }
    }
    void YOLO::detect()
    {
        Detector detector("weights/yolov7-tiny.cfg", "weights/yolov7-tiny.weights");
        cv::Mat local_frame;
        while(true)
        {
            int64 t = cv::getTickCount();

            cam_->mtx_.lock();
            cam_->frame.copyTo(local_frame);
            cam_->mtx_.unlock();
            std::vector<bbox_t> result_vec = detector.detect(local_frame);
            
            for (auto &i : result_vec) {
                cv::rectangle(local_frame, cv::Rect(i.x, i.y, i.w, i.h), cv::Scalar(50, 200, 50), 3);
            }

            texture_->mtx_.lock();
            texture_->height = local_frame.rows;
            texture_->width = local_frame.cols;
            texture_->image = local_frame.data;
            if(texture_->dirty){texture_->dirty = false;};
            texture_->mtx_.unlock();    
            t = cv::getTickCount() - t;
            
            objs_->mtx_.lock();
            objs_->obj_array.clear();
            for (auto &i : result_vec) {
                objs_->obj_array.push_back(machycore::yolo_obj(
                    i.obj_id, i.x, i.y, i.w, i.h, i.prob
                ));
            }
            objs_->mtx_.unlock();     
        }
    }
}