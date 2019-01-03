#ifndef TESTFACEAPI_TESTFACEAPI_LIVENESS_H
#define TESTFACEAPI_TESTFACEAPI_LIVENESS_H
#include "baidu_face_api.h"
#include "opencv2/opencv.hpp"
// �������Ϊ�޻���ɼ����л���ɼ�(����RGB��Ŀ���弰RGB+IR˫Ŀ����)
class Liveness
{
public:
    Liveness();
    ~Liveness();
public:
    // usb����ͷ��⣬ʵʱ���������Ϣ
    void usb_track_face_info(BaiduFaceApi *api);
    // ͨ��ͼƬ��⣬�������������Ϣ
    void image_track_face_info(BaiduFaceApi *api);
    // ͨ��ͼƬ���(���������ͼƬbuf�����������������Ϣ
    void image_track_face_info_by_buf(BaiduFaceApi *api);
    
};

#endif
