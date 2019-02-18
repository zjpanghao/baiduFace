#include "test_face.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <regex>
#include <sstream>
#include <fstream>
#include <iterator>
#include <thread>
#include "opencv2/opencv.hpp"
#include "baidu_face_api.h"
#include "image_base64.h"
#include "cv_help.h"
#include "json/json.h"
#include "liveness.h"
#include "compare.h"
#include "setting.h"
#include <chrono>
#include "image_buf.h"
#include <sys/time.h>
using namespace cv;
using namespace std;

// ��ȡ����ʱ���
std::time_t get_timestamp()
{
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::time_t timestamp = tmp.count(); 
    return timestamp;
}

// ������������
void test_face_attr(BaiduFaceApi *api)
{
    // ��ȡ�������ԣ�ͨ������ͼƬbase64(1:base64ͼƬ��2ΪͼƬ·����
    std::string base64 = ImageBase64::file2base64("2.jpg");
    std::string res1 = api->face_attr(base64.c_str(), 1);
    std::cout<<"attr res1 is:"<<res1<<std::endl; 
   // ��ȡ�������ԣ�ͨ������ͼƬ·��(1:base64ͼƬ��2ΪͼƬ·����
    std::cout<<"before face_attr"<<std::endl;
    std::string res2 = api->face_attr("2.jpg",2);
    std::cout<<"attr res2 is:"<<res2<<std::endl;  
    std::string out_buf;
    // ��ȡ�ļ���������buffer
    int buf_len = ImageBuf::get_buf("2.jpg", out_buf);
    std::string res_buf = api->face_attr_by_buf((unsigned char*)out_buf.c_str(), buf_len);
    std::cout<<"res_buf is:"<<res_buf<<std::endl;
}

// ������������
void test_face_quality(BaiduFaceApi *api)
{
    // ��ȡ�������ԣ�ͨ������ͼƬbase64(1:base64ͼƬ��2ΪͼƬ·����
    // api->set_min_face_size(30);
    std::string base64 = ImageBase64::file2base64("1.jpg");
    std::string res1 = api->face_quality(base64.c_str(), 1);
    std::cout << "res1 is:"<< res1 << std::endl;
    // ��ȡ�������ԣ�ͨ������ͼƬ·��(1:base64ͼƬ��2ΪͼƬ·����
    std::string res2 = api->face_quality("1.jpg", 2);
    std::cout << "res2 is:"<< res2 << std::endl;
    // ��ȡ�������ԣ�ͨ������ͼƬ������buffer
    std::string out_buf;
    // ��ȡ�ļ���������buffer
    std::cout << "in test_face_quality" << std::endl;
    int buf_len = ImageBuf::get_buf("1.jpg", out_buf);
    std::cout << "in test_face_quality 1111" << std::endl;
    std::string res_buf = api->face_quality_by_buf((unsigned char*)out_buf.c_str(), buf_len); 
    std::cout << "res_buf is:" << res_buf << std::endl;
}

//���Ի�ȡ�豸ָ��device_id
void test_get_device_id(BaiduFaceApi *api)
{
    std::string res = api->get_device_id();
    std::cout << "---res is:" << res << std::endl;
}

// �Ƿ���Ȩ
void test_is_auth(BaiduFaceApi *api)
{
    bool authed = api->is_auth();
    if (authed)
    {
        std::cout << "authed is true" << std::endl;
    }
    else
    {
        std::cout << "authed is false" << std::endl;
    }
}

// ͨ���޸�setting����Ķ�Ӧ���������ﵽ�������������Ե�Ŀ��
void test_face_setting(BaiduFaceApi* api)
{
    Setting *setptr = new Setting();
    // setptr->set_blur_thr(api);
    // setptr->set_detect_in_video_interval(api);
    // setptr->set_eulur_angle_thr(api);
    // setptr->set_illum_thr(api);
    // setptr->set_is_check_quality(api);
    // ������С����
    setptr->set_min_face_size(api);
    //  setptr->set_notface_thr(api);
    //  setptr->set_occlu_thr(api);
    //  setptr->set_track_by_detection_interval(api);
    delete setptr;
}

// ���Աȶԣ�1:1�����ȶԣ�1��N�����ȶ�,����ֵ�ȶԵ�
void test_compare(BaiduFaceApi* api)
{
    Compare *comptr = new Compare();
    //1:1�Ա�
    // comptr->match(api);
   
    //ͼƬ����Ƶ�Ա�
   // comptr->match_by_img_and_frame(api, "1.jpg");
    //����Ϊ����ֵ����demo
    // comptr->get_face_feature_by_img(api);
   // comptr->get_face_feature_by_frame(api);
    // comptr->get_face_feature_by_buf(api);
     //����ֵ�Ƚ�
     comptr->compare_feature(api);
    // ���Ի�ȡ����ֵ������ֵ�Ƚ�(�Ƚ��ۺϣ��ɸ��ݲ�����ͬ�Ƚ�)
    //comptr->get_face_feature_and_match(api);
    delete comptr;
}

// �ɼ������
void test_liveness_track(BaiduFaceApi* api)
{
    Liveness* liveptr = new Liveness();
   
     // usb����ͷ���������ɼ�
   //  liveptr->usb_track_face_info(api);
   
    // �Ե���ͼƬ���м��������Ϣ 
    //liveptr->image_track_face_info(api);
    // �Զ�����ͼƬ���м��������Ϣ
    liveptr->image_track_face_info_by_buf(api);
     delete liveptr;
}

// opencv��rtsp������Ƶʾ������
void test_opencv_video()
{
    // rtsp��������ͷ��ַ
    cv::VideoCapture cap("rtsp://admin:Aa123456@192.168.1.65:554/h264/ch1/main/av_stream");
    if (!cap.isOpened())
    {
        std::cout << "open camera error" << std::endl;
        return;
    }
    cv::Mat frame;
    bool stop = false;
    int index = 0;
   
    while (!stop)
    {
        cap >> frame;
     /*   if (!frame.empty())
        {
            imshow("face", frame);
            cv::waitKey(1);
            std::cout << "mat not empty" << std::endl;
        }
        else
        {
            std::cout << "mat is empty" << std::endl;
        } */

        imshow("face", frame);
        cv::waitKey(1);
      //  frame.release();
    }
}

// ������ں���
int main(int argc, char*argv[])
{
    std::cout << "in main" << std::endl;

  //  test_opencv_video();
    //�ٶ�����ʶ��sdk apiʵ��ָ��
    BaiduFaceApi *api = new BaiduFaceApi();
   
    // sdk��ʼ��,����֤����ģʽ�����id_card��Ϊtrue������Ϊfalse��
    // ֤����ģʽ�ͷ�֤����ģʽ��ȡ����������ֵ��ͬ�����ܻ���
    std::cout<<"in main init"<<std::endl;
    bool id_card = false;
    api->sdk_init(id_card);
    api->set_min_face_size(1);
    std::cout<<"in main after init"<<std::endl;
    std::time_t time_begin = get_timestamp();
    // ʱ���
    std::cout<<"time begin is:"<<time_begin<<std::endl;
   // test_face_setting(api);

    std::string res = "";
   
    // ������������
    // test_face_attr(api);
    // ����ͼƬ���� 
   // test_face_quality(api);
    std::vector<TrackFaceInfo> *info = new std::vector<TrackFaceInfo>();
    int n = api->track(info, argv[1], 2, 20);
    std::cout << "detect :" << n << std::endl;
    // ���ԶԱ�
    //test_compare(api);
    // �����������
    //test_liveness_track(api);
   // ���Ի�ȡ�豸id
   // test_get_device_id(api);
   // �����Ƿ���Ȩ
    test_is_auth(api);
    std::time_t time_end = get_timestamp();
    std::cout<<"time end is:"<<time_end<<std::endl;
    std::cout << "time  cost is :" << time_end - time_begin <<" ms"<< std::endl;
    return 0;
    delete api;
   // getchar();
    return 0;
}

TestFace::TestFace()
{
}

TestFace::~TestFace()
{
}
