#ifndef TESTFACEAPI_TESTFACEAPI_COMPARE_H
#define TESTFACEAPI_TESTFACEAPI_COMPARE_H
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
class BaiduFaceApi;
// �������Ϊ�����ȶ��࣬����1:1�����ȶԣ�1��N�����ȶԵ�
class Compare
{
public:
    Compare();
    ~Compare();
public:
    //1:1 �����ԱȽӿ�
    void match(BaiduFaceApi *api);
    //��Ƶ֡��ͼƬ1:1�Ա�
    void match_by_img_and_frame(BaiduFaceApi *api, const std::string& path);
    // ��ȡ��������ֵ(ͨ��ͼƬ)
    void get_face_feature_by_img(BaiduFaceApi *api);
    // ��ȡ��������ֵ(ͨ����Ƶ֡)
    void get_face_feature_by_frame(BaiduFaceApi *api);
    // ��ȡ��������ֵ(���������ͼƬbuffer)
    void get_face_feature_by_buf(BaiduFaceApi *api);
    // ��ȡ����ֵ��������ֵ��ͼƬ�Ƚϵ��ۺ�ʾ��
    void get_face_feature_and_match(BaiduFaceApi *api);
    //����ֵ�ȶ�
    void compare_feature(BaiduFaceApi *api);
private:
    void many_images(std::vector<cv::Mat> images, cv::Mat& dst, int img_rows);
    
    float parse_score(const std::string& res);
};

#endif
