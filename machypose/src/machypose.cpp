#include "machypose.h"

namespace machypose
{
    void ORBSLAM::detectMonocular()
    {
        int width_img, height_img;
        double timestamp_image = -1.0;
        int count_im_buffer = 0; // count dropped frames
        
        string Vocabpath = "libs/ORB_SLAM3/Vocabulary/ORBvoc.txt"
        string Settingspath = "./libs/ORB_SLAM3/Examples/Monocular/Realsense_D435i.yaml"
        // Create SLAM system. It initializes all system threads and gets ready to process frames.
        ORB_SLAM3::System SLAM(Vocabpath, Settingspath, ORB_SLAM3::System::MONOCULAR, false, 0);
        float imageScale = SLAM.GetImageScale();

        double timestamp;
        cv::Mat im;
        while (!SLAM.isShutDown())
        {
            count_im_buffer++;
            double new_timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            if (abs(timestamp_image-new_timestamp)<0.001){
                count_im_buffer--;
            }
            cam_->mtx_.lock();
            cam_->frame.copyTo(im);
            cam_->mtx_.unlock();

            timestamp_image = std::chrono::system_clock::now().time_since_epoch().count();
            timestamp = timestamp_image * 1e-3;

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
            std::cout << "x: " << twc(0) << " y: " << twc(1) << " z: " << twc(2) << " rx: " q.x() << " ry: " << q.y() << " rz: " << q.z() << std::endl;
        }
        cout << "System shutdown!\n";
    }
}