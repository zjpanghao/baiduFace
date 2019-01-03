#ifndef TESTFACEAPI_TESTFACEAPI_SET_SETTING_H
#define TESTFACEAPI_TESTFACEAPI_SET_SETTING_H
#include "opencv2/opencv.hpp"
#include "baidu_face_api.h"

// �������Ϊ�������������ã������ü����С������С�����ù��ա�ģ���ȵȲ���
class Setting
{
public:
    Setting();
    ~Setting();
public:
    // �����Ƿ�ִ��������⣬Ĭ��Ϊfalse
    void set_is_check_quality(BaiduFaceApi *api);
    // ������ֵ��ȡֵ��Χ0~255��Ĭ��40��Խ���������Խ��
    void set_illum_thr(BaiduFaceApi *api);
    // ģ����ֵ��ȡֵ��Χ0~1��Ĭ��0.7��ԽС����ͼ��Խ����
    // ������Ϊ1ʱ��ģ����ⲻ�����ҽ����ͨ����������Ϊ0ʱ��ģ����ⲻ�����ҽ��ͨ��
    void set_blur_thr(BaiduFaceApi *api);
    // ���ۡ����ۡ����ӡ���͡������ա������ա��°͵��ڵ���ֵ��ȡֵ��Χ0~1��Ĭ��0.5��ֵԽС�����ڵ�����ԽС
    // ������Ϊ1ʱ���ڵ���ⲻ�����ҽ����ͨ����������Ϊ0ʱ���ڵ���ⲻ�����ҽ��ͨ��
    void set_occlu_thr(BaiduFaceApi *api);
    // ����pitch��yaw��roll�ȽǶȵ���ֵ��Ĭ�϶�Ϊ15&deg;��Խ�����ɲɼ��������Ƕ�Խ��
    // ���ǽǶ�Խ��ʶ��Ч����Խ��
    void set_eulur_angle_thr(BaiduFaceApi *api);
    // �����������Ŷ���ֵ��ȡֵ��Χ0~1��ȡ0����Ϊ���м������Ľ������������Ĭ��0.5
    void set_notface_thr(BaiduFaceApi *api);
    // ��С�����ߴ磺��Ҫ��⵽����С�����ߴ磬������Ҫ��⵽30*30������������Ϊ30��
    // �óߴ�ռͼ�����ԽС�����ٶ�Խ����������ο�����ָ���½ڡ�Ĭ��ֵ30
    void set_min_face_size(BaiduFaceApi *api);
    // ���ٵ�Ŀ���ִ�м���ʱ��������λ���룬Ĭ��300��ֵԽСԽ����췢����Ŀ�꣬
    // ����cpuռ���ʻ���ߡ������ٶȱ���
    void set_track_by_detection_interval(BaiduFaceApi *api);
    // δ���ٵ�Ŀ��ʱ�ļ��������λ���룬Ĭ��300��ֵԽСԽ�췢����Ŀ�꣬
    // ����cpuռ���ʻ���ߡ������ٶȱ���
    void set_detect_in_video_interval(BaiduFaceApi *api);
};

#endif
