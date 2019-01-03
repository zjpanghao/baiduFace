#ifndef TESTFACEAPI_TESTFACEAPI_SET_SETTING_H
#define TESTFACEAPI_TESTFACEAPI_SET_SETTING_H
#include "opencv2/opencv.hpp"
#include "baidu_face_api.h"

// 该类可作为基础的人脸设置，如设置检测最小人脸大小，设置光照、模糊度等参数
class Setting
{
public:
    Setting();
    ~Setting();
public:
    // 设置是否执行质量检测，默认为false
    void set_is_check_quality(BaiduFaceApi *api);
    // 光照阈值，取值范围0~255，默认40，越大代表脸部越亮
    void set_illum_thr(BaiduFaceApi *api);
    // 模糊阈值，取值范围0~1，默认0.7，越小代表图像越清晰
    // 当设置为1时，模糊检测不进行且结果不通过；当设置为0时，模糊检测不进行且结果通过
    void set_blur_thr(BaiduFaceApi *api);
    // 右眼、左眼、鼻子、嘴巴、左脸颊、右脸颊、下巴的遮挡阈值，取值范围0~1，默认0.5，值越小代表遮挡程序越小
    // 当设置为1时，遮挡检测不进行且结果不通过；当设置为0时，遮挡检测不进行且结果通过
    void set_occlu_thr(BaiduFaceApi *api);
    // 设置pitch、yaw、roll等角度的阈值，默认都为15&deg;，越大代表可采集的人脸角度越大，
    // 但是角度越大识别效果会越差
    void set_eulur_angle_thr(BaiduFaceApi *api);
    // 非人脸的置信度阈值，取值范围0~1，取0则认为所有检测出来的结果都是人脸，默认0.5
    void set_notface_thr(BaiduFaceApi *api);
    // 最小人脸尺寸：需要检测到的最小人脸尺寸，比如需要检测到30*30的人脸则设置为30，
    // 该尺寸占图像比例越小则检测速度越慢，具体请参考性能指标章节。默认值30
    void set_min_face_size(BaiduFaceApi *api);
    // 跟踪到目标后执行检测的时间间隔，单位毫秒，默认300，值越小越会更快发现新目标，
    // 但是cpu占用率会提高、处理速度变慢
    void set_track_by_detection_interval(BaiduFaceApi *api);
    // 未跟踪到目标时的检测间隔，单位毫秒，默认300，值越小越快发现新目标，
    // 但是cpu占用率会提高、处理速度变慢
    void set_detect_in_video_interval(BaiduFaceApi *api);
};

#endif
