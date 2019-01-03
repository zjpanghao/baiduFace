#ifndef TESTFACEAPI_TESTFACEAPI_LIVENESS_H
#define TESTFACEAPI_TESTFACEAPI_LIVENESS_H
#include "baidu_face_api.h"
#include "opencv2/opencv.hpp"
// 该类可作为无活体采集及有活体采集(包括RGB单目活体及RGB+IR双目活体)
class Liveness
{
public:
    Liveness();
    ~Liveness();
public:
    // usb摄像头检测，实时输出人脸信息
    void usb_track_face_info(BaiduFaceApi *api);
    // 通过图片检测，输出检测的人脸信息
    void image_track_face_info(BaiduFaceApi *api);
    // 通过图片检测(传入二进制图片buf），输出检测的人脸信息
    void image_track_face_info_by_buf(BaiduFaceApi *api);
    
};

#endif
