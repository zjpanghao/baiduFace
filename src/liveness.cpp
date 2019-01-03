
#include "liveness.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <regex>
#include <sstream>
#include <fstream>
#include <iterator>
#include <thread>
#include "baidu_face_api.h"
#include "image_base64.h"
#include "cv_help.h"
#include "image_buf.h"
#include "json/json.h"
Liveness::Liveness()
{
}

Liveness::~Liveness()
{
}


// usb摄像头人脸检测
void Liveness::usb_track_face_info(BaiduFaceApi *api)
{
   
    int device = 0; // 默认检测摄像头为0,若不对，可依次修改为1-10,实际上为您设备接上摄像头后认定的图像设备id
    cv::VideoCapture cap(device);
    if (!cap.isOpened())
    {
        std::cout << "open camera error" << std::endl;
        return;
    }
    cv::Mat frame;
    bool stop = false;
    int index = 0;
    bool save_file = true; //可选择是否保存检测到的图片
    cv::RotatedRect box;
    std::vector<TrackFaceInfo> *track_info = new std::vector<TrackFaceInfo>();
    while (!stop)
    {
        cap >> frame;
        track_info->clear();
        
        int size = api->track(track_info, frame, 1);
        for (int i = 0; i < size; i++)
        {
            TrackFaceInfo info = track_info->at(i);
            std::cout << "in Liveness::usb_track score is:" << info.score << std::endl;
            // 人脸框信息
            FaceInfo ibox = info.box;
            // 角度
            std::cout << "in Liveness::usb_track mAngle is:" << ibox.mAngle << std::endl;
            // 人脸宽度
            std::cout << "in Liveness::usb_track mWidth is:" << ibox.mWidth << std::endl;
            // 中心点x，y坐标
            std::cout << "in Liveness::usb_track mCenter_x is:" << ibox.mCenter_x << std::endl;
            std::cout << "in Liveness::usb_track mCenter_y is:" << ibox.mCenter_y << std::endl;
            std::vector<float> headPose = info.headPose;
            // 返回x，y，z三个角度的人脸角度
            for (int k = 0; k < headPose.size(); k++)
            {
                float angle = headPose.at(k);
                std::cout << "in Liveness::usb_track angle is:" << angle << std::endl;
            }
            //画人脸框
            box = CvHelp::bounding_box(info.landmarks);
            CvHelp::draw_rotated_box(frame, box, cv::Scalar(0, 255, 0));
            // 画关键点轮廓
            CvHelp::draw_shape(info.landmarks, frame, cv::Scalar(0, 255, 0));

            // frame为视频帧，可根据采集到的人脸信息筛选需要的帧
            // 以下为保存图片到本地的示例，可根据采集信息有选择的保存
            if (!save_file)
            {
                save_file = true;
                cv::imwrite("usb_track.jpg", frame);
            }

        } 
        if (!frame.empty())
        {
            imshow("face", frame);
            cv::waitKey(1);
            std::cout << "mat not empty" << std::endl;
        }
        else
        {
            std::cout << "mat is empty" << std::endl;
        }
        frame.release();
       
    }
    delete track_info;
}


void Liveness::image_track_face_info(BaiduFaceApi *api)
{
   
    cv::RotatedRect box;
    std::vector<TrackFaceInfo> *track_info = new std::vector<TrackFaceInfo>();
    std::string img_path = "2.jpg";
    int size = api->track(track_info, img_path.c_str(), 2,1);

    if (size > 0)
    {
        for (int i = 0; i < size; i++)
	{
	    TrackFaceInfo info = track_info->at(i);
	    std::cout << "in Liveness::image_track_face_info score is:" << info.score << std::endl;
	    // 人脸框信息
	    FaceInfo ibox = info.box;
	    // 角度
	    std::cout << "in Liveness::image_track_face_info mAngle is:" << ibox.mAngle << std::endl;
	    // 人脸宽度
	    std::cout << "in Liveness::image_track_face_info mWidth is:" << ibox.mWidth << std::endl;
	    // 中心点X，Y坐标
	    std::cout << "in Liveness::image_track_face_info mCenter_x is:" << ibox.mCenter_x << std::endl;
	    std::cout << "in Liveness::image_track_face_info mCenter_y is:" << ibox.mCenter_y << std::endl;
	    std::vector<float> headPose = info.headPose;
	    // 返回x y z三个角度的人脸角度
	    for (int k = 0; k < headPose.size(); k++)
	    {
                float angle = headPose.at(k);
		std::cout << "in Liveness::image_track_face_info angle is:" << angle << std::endl;
	    }
	    // 画人脸框
	   // box = CvHelp::bounding_box(info.landmarks);
	   // CvHelp::draw_rotated_box(frame, box, cv::Scalar(0, 255, 0));
	    // 画关键点轮廓
	    //CvHelp::draw_shape(info.landmarks, frame, cv::Scalar(0, 255, 0));
	}
    }
    else
    {
	std::cout << "image has no face!" << std::endl;
    }

	/*if (!frame.empty())
	{
	    std::cout << "11111!" << std::endl;
	    cv::namedWindow("face",CV_WINDOW_AUTOSIZE);
	    std::cout << "22222!" << std::endl;
	   // imshow("face", frame);
	    cv::Mat mat = cv::imread("1.jpg");
	    imshow("face",mat);
	    std::cout << "3333!" << std::endl;
	    cv::waitKey(3);
	    std::cout << "mat not empty" << std::endl;
	}
	else
	{
	    std::cout << "mat is empty" << std::endl;
	} */
   // frame.release();
    
    delete track_info;
}
// 通过图片检测，输出人脸信息(传入二进制图片buffer）
void Liveness::image_track_face_info_by_buf(BaiduFaceApi *api)
{
    std::string out_buf;
    int buf_len = ImageBuf::get_buf("1.jpg", out_buf);

    cv::RotatedRect box;
    std::vector<TrackFaceInfo> *track_info = new std::vector<TrackFaceInfo>();

   // track_info->clear();
   int size = 0;
   api->set_min_face_size(10);
   for (int i = 0; i < 1; i++) {
     size = api->track_by_buf(track_info, (unsigned char*)out_buf.c_str(), buf_len, 5);
   }
    if (size > 0)
    {
        for (int i = 0; i < size; i++)
        {
            TrackFaceInfo info = track_info->at(i);
            std::cout << "in Liveness::image_track_face_info score is:" << info.score << std::endl;
            // 人脸信息
            FaceInfo ibox = info.box;
            // 角度
            std::cout << "in Liveness::image_track_face_info mAngle is:" << ibox.mAngle << std::endl;
            // 人脸宽度
            std::cout << "in Liveness::image_track_face_info mWidth is:" << ibox.mWidth << std::endl;
            // 中心点X,Y坐标
            std::cout << "in Liveness::image_track_face_info mCenter_x is:" << ibox.mCenter_x << std::endl;
            std::cout << "in Liveness::image_track_face_info mCenter_y is:" << ibox.mCenter_y << std::endl;
            std::vector<float> head_pose = info.headPose;
            // 返回 x，y，z三个角度的人脸角度
            for (int k = 0; k < head_pose.size(); k++)
            {
                float angle = head_pose.at(k);
                std::cout << "in Liveness::image_track_face_info angle is:" << angle << std::endl;
            }
           
        }
    }
    else
    {
        std::cout << "image has no face!" << std::endl;
    }
    cv::waitKey(1);
    delete track_info;
}

