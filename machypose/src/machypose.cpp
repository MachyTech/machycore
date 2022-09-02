#include "machypose.h"

namespace machypose
{

    auto ClosestTargetPoint(std::vector<cv::KeyPoint> KP, std::vector<ORB_SLAM3::MapPoint*> MP, int x, int y, int w, int h){
        struct Values{
            bool value1;
            ORB_SLAM3::MapPoint* value2;
        };

        float check = 1000000000;
        bool istarget = false;
        ORB_SLAM3::MapPoint* TargetPoint;
        for (int i=0; i<KP.size();i++){
            if (KP[i].pt.x < (x+w) & KP[i].pt.x > x & KP[i].pt.y < (y+h) & KP[i].pt.y > y){
                float targetdist = sqrt(pow(240-KP[i].pt.x,2)+pow(320-KP[i].pt.y,2));
                if (targetdist < check){
                    istarget = true;
                    TargetPoint = MP[i];
                }
            }
        }
        return Values{istarget, TargetPoint};
    }

    std::vector<cv::KeyPoint> GetMatchedKeyPoints(std::vector<cv::KeyPoint> KP, std::vector<ORB_SLAM3::MapPoint*> MP){
        std::vector<bool> vbKP = std::vector<bool>(KP.size(),false);
        std::vector<bool> vbMP = std::vector<bool>(KP.size(),false);
        std::vector<cv::KeyPoint> Matches;
        for (int i=0; i<KP.size();i++){
            ORB_SLAM3::MapPoint* pMP = MP[i];
            if (pMP)
                if (pMP->Observations()>0)
                    vbMP[i]=true;
                else
                    vbKP[i]=true;
            if(vbKP[i] || vbMP[i])
            {
                // This is a match to a MapPoint in the map
                if(vbMP[i])
                {   
                    Matches.push_back(KP[i]);
                }
                else // This is match to a "visual odometry" MapPoint created in the last frame
                {
                    Matches.push_back(KP[i]);
                }

            }
        }
        return Matches;
    }

    std::vector<ORB_SLAM3::MapPoint*> GetMatchedMapPoints(std::vector<cv::KeyPoint> KP, std::vector<ORB_SLAM3::MapPoint*> MP){
        std::vector<bool> vbKP = std::vector<bool>(KP.size(),false);
        std::vector<bool> vbMP = std::vector<bool>(KP.size(),false);
        std::vector<ORB_SLAM3::MapPoint*> Matches;
        for (int i=0; i<KP.size();i++){
            ORB_SLAM3::MapPoint* pMP = MP[i];
            if (pMP)
                if (pMP->Observations()>0)
                    vbMP[i]=true;
                else
                    vbKP[i]=true;
            if(vbKP[i] || vbMP[i])
            {
                // This is a match to a MapPoint in the map
                if(vbMP[i])
                {   
                    Matches.push_back(pMP);
                }
                else // This is match to a "visual odometry" MapPoint created in the last frame
                {
                    Matches.push_back(pMP);
                }
            }
        }
        return Matches;
    }

    void ORBSLAM::detectMonocular()
    {      
        string Vocabpath = "libs/ORB_SLAM3/Vocabulary/ORBvoc.txt";
        string Settingspath = "./libs/ORB_SLAM3/Examples/Monocular/Realsense_D435i.yaml";
        ORB_SLAM3::System SLAM(Vocabpath, Settingspath, ORB_SLAM3::System::MONOCULAR, false, 0);
        float imageScale = SLAM.GetImageScale();

        cv::Mat im;
        while (!SLAM.isShutDown())
        {
            cam_->mtx_.lock();
            cam_->frame.copyTo(im);
            long timestamp = cam_->timestamp;
            cam_->mtx_.unlock();

            yolo_data_->mtx_.lock();
            std::vector<machycore::yolo_obj> VisionData = yolo_data_->obj_array;
            yolo_data_->mtx_.unlock();
            
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

            // Retrieve all Keypoints and Mappoints from last frame.
            std::vector<cv::KeyPoint> KP = SLAM.GetTrackedKeyPoints();
            std::vector<ORB_SLAM3::MapPoint*> MP = SLAM.GetTrackedMapPoints();
            
            // Retrieve all tracked Keypoints and Mappoints in latest frames.
            std::vector<cv::KeyPoint> KP_Matches = GetMatchedKeyPoints(KP, MP);
            std::vector<ORB_SLAM3::MapPoint*> MP_Matches = GetMatchedMapPoints(KP, MP);   
            
            // Find world pos of pixel closest to center of bounding box.
            std::vector<std::variant<ORB_SLAM3::MapPoint*, bool>> Targets
            for (auto &i : VisionData){
                auto [istarget, Target] = ClosestTargetPoint(KP, MP, i.x, i.y, i.width, i.height);
                if (istarget){
                    Targets.push_back(Target);
                }
                else{
                    Targets.push_back(false);
                }
            }

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

            feature_data_->mtx_.lock();
            feature_data_->feature_array.clear();
            for (auto &i : KP_Matches) {
                feature_data_->feature_array.push_back(machycore::feature_obj(
                    i.pt.x, i.pt.y
                ));
            }
            feature_data_->mtx.unlock();

            pose_object_data_->mtx_.lock();
            pose_object_data_->pose_object_array.clear();
            for (auto &i : Targets){
                if (i==true){
                    pose_object_data_->pose_object_array.push_back(machycore::pose_object_obj(
                        i->GetWorldPos()[0], i->GetWorldPos()[1], i->GetWorldPos()[2]
                    ));
                }
                else{
                    pose_object_data->pose_object_array.push_back(machycore::pose_object_obj(

                    ))
                }
            }
            pose_object_data_->mtx_.unlock();
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
            long timestamp -> cam_.timestamp;
            cam_->mtx_.unlock();
            
            cam2_->mtx_.lock();
            cam2_->frame.copyTo(imRight);
            long timestamp2 -> cam2_.timestamp;
            cam2_->mtx_.unlock();

            yolo_data_->mtx_.lock();
            std::vector<machycore::yolo_obj> VisionData = yolo_data_->obj_array;
            yolo_data_->mtx_.unlock();

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
            
            // Retrieve all Keypoints and Mappoints from last frame.
            std::vector<cv::KeyPoint> KP = SLAM.GetTrackedKeyPoints();
            std::vector<ORB_SLAM3::MapPoint*> MP = SLAM.GetTrackedMapPoints();
            
            // Retrieve all tracked Keypoints and Mappoints in latest frames.
            std::vector<cv::KeyPoint> KP_Matches = GetMatchedKeyPoints(KP, MP);
            std::vector<ORB_SLAM3::MapPoint*> MP_Matches = GetMatchedMapPoints(KP, MP);   
            
            // Find world pos of pixel closest to center of bounding box.
            std::vector<ORB_SLAM3::MapPoint*> Targets
            for (auto &i : VisionData){
                auto [istarget, Target] = ClosestTargetPoint(KP, MP, i.x, i.y, i.w, i.h);
                if (istarget){
                    Targets.push_back(Target);
                }
            }

            texture_->mtx_.lock();
            texture_->height = im.rows;
            texture_->width = im.cols;
            texture_->image = im.data;
            if(texture_->dirty){
                texture_->dirty = false;
            }
            texture_->mtx_.unlock();

            pose_data_->mtx_.lock();
            pose_data_->pose_array.clear();
            pose_data_->pose_array.push_back(machycore::pose_obj(twc(0), twc(1), twc(2), q.x(), q.y(), q.z(), q.w()));
            pose_data_->mtx_.unlock();

            feature_data_->mtx_.lock();
            feature_data_->feature_array.clear();
            for (auto &i : KP_Matches) {
                feature_data_->feature_array.push_back(machycore::feature_obj(
                    i.pt.x, i.pt.y
                ));
            }
            feature_data_->mtx.unlock();

            pose_object_data_->mtx_.lock();
            pose_object_data_->pose_object_array.clear();
            for (auto &i : Targets){
                pose_object_data_->pose_object_array.push_back(machycore::pose_object_obj(
                    i->GetWorldPos()[0], i->GetWorldPos()[1], i->GetWorldPos()[2]
                ));
            }
            pose_object_data_->mtx_.unlock();
        }
        std::cout << "System shutdown!\n";
    }
}