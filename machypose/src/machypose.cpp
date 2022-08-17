#include "machypose.h"

namespace machypose
{
    void ORBSLAM::detectMonocular()
    {      
        string Vocabpath = "libs/ORB_SLAM3/Vocabulary/ORBvoc.txt";
        string Settingspath = "./libs/ORB_SLAM3/Examples/Monocular/Realsense_D435i.yaml";
        // Create SLAM system. It initializes all system threads and gets ready to process frames.
        ORB_SLAM3::System SLAM(Vocabpath, Settingspath, ORB_SLAM3::System::MONOCULAR, false, 0);
        float imageScale = SLAM.GetImageScale();

        cv::Mat im;
        while (!SLAM.isShutDown())
        {
            cam_->mtx_.lock();
            cam_->frame.copyTo(im);
            cam_->mtx_.unlock();
            
            long timestamp = cam_->timestamp;

            if(imageScale != 1.f)
            {
                int width = im.cols * imageScale;
                int height = im.rows * imageScale;
                cv::resize(im, im, cv::Size(width, height));
            }
            // Stereo images are already rectified.
            Sophus::SE3f Tcw = SLAM.TrackMonocular(im, timestamp);
            Sophus::SE3f Twc = Tcw.inverse();
            Eigen::Quaternionf q = Twc.unit_quaternion();
            Eigen::Vector3f twc = Twc.translation();

            texture_->mtx_.lock();
            texture_->height = im.rows;
            texture_->width = im.cols;
            texture_->image = im.data;
            if(texture_->dirty){texture_->dirty = false;};
            texture_->mtx_.unlock();

            pose_data_->mtx_.lock();
            pose_data_->pose_array.clear();
            pose_data_->pose_array.push_back(machycore::pose_obj(twc(0), twc(1), twc(2), q.x(), q.y(), q.z(), q.w()));
            pose_data_->mtx_.unlock();
        }
        cout << "System shutdown!\n";
    }

    void ORBSLAM::detectStereo()
    {
        string Vocabpath = "libs/ORB_SLAM3/Vocabulary/ORBvoc.txt";
        string Settingspath = "./libs/ORB_SLAM3/Examples/Stereo/Realsense_D435i.yaml";
        ORB_SLAM3::System SLAM(Vocabpath, Settingspath, ORB_SLAM3::System::STEREO, false, 0);
        float imageScale = SLAM.GetImageScale();

        cv::Mat im;
        cv::Mat imRight;
        while(!SLAM.isShutDown())
        {
            cam_->mtx_.lock();
            cam_->frame.copyTo(im);
            cam_->mtx_.unlock();
            
            cam2_->mtx_.lock();
            cam2_->frame.copyTo(imRight);
            cam2_->mtx_.unlock();

            long timestamp = cam_->timestamp;

            if (imageScale != 1.f)
            {
                int width = im.cols * imageScale;
                int height = im.rows * imageScale;
                cv::resize(im, im, cv::Size(width, height));
                cv::resize(imRight, imRight, cv::Size(width, height));
            }
            Sophus::SE3f Tcw = SLAM.TrackStereo(im, imRight, timestamp);
            Sophus::SE3f Twc = Tcw.inverse();
            Eigen::Quaternionf q = Twc.unit_quaternion();
            Eigen::Vector3f twc = Twc.translation();

            texture_->mtx_.lock();
            texture_->height = im.rows;
            texture_->width = im.cols;
            texture_->image = im.data;
            if(texture_->dirty){texture_->dirty = false;};
            texture_->mtx_.unlock();

            pose_data_->mtx_.lock();
            pose_data_->pose_array.clear();
            pose_data_->pose_array.push_back(machycore::pose_obj(twc(0), twc(1), twc(2), q.x(), q.y(), q.z(), q.w()));
            pose_data_->mtx_.unlock();
        }
        std::cout << "System shutdown!\n";
    }
}