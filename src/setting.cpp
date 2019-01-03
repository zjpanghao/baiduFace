
#include "setting.h"

/**
*  该类可作为基础的人脸设置，如设置检测最小人脸大小，设置光照、模糊度等参数
*  用户可通过修改该类的参数值，来达到调整人脸检测、识别参数的目的。
*/
Setting::Setting()
{
}

Setting::~Setting()
{
}
// 设置是否执行质量检测，默认为false
void Setting::set_is_check_quality(BaiduFaceApi *api)
{
    api->set_isCheckQuality(true);
}
// 光照阈值，取值范围0~255，默认40，越大代表脸部越亮
void Setting::set_illum_thr(BaiduFaceApi *api)
{
    api->set_illum_thr(40);
}
// 模糊阈值，取值范围0~1，默认0.7，越小代表图像越清晰
// 当设置为1时，模糊检测不进行且结果不通过；当设置为0时，模糊检测不进行且结果通过
void Setting::set_blur_thr(BaiduFaceApi *api)
{
    api->set_blur_thr(0.7);
}

// 右眼、左眼、鼻子、嘴巴、左脸颊、右脸颊、下巴的遮挡阈值，取值范围0~1，默认0.5，值越小代表遮挡程序越小
// 当设置为1时，遮挡检测不进行且结果不通过；当设置为0时，遮挡检测不进行且结果通过
void Setting::set_occlu_thr(BaiduFaceApi *api)
{
    api->set_occlu_thr(0.5);
}
// 设置pitch、yaw、roll等角度的阈值，默认都为15&deg;，越大代表可采集的人脸角度越大，
// 但是角度越大识别效果会越差
void Setting::set_eulur_angle_thr(BaiduFaceApi *api)
{
    api->set_eulur_angle_thr(15, 15, 15);
}
// 非人脸的置信度阈值，取值范围0~1，取0则认为所有检测出来的结果都是人脸，默认0.5
void Setting::set_notface_thr(BaiduFaceApi *api)
{
    api->set_notFace_thr(0.5);
}
// 最小人脸尺寸：需要检测到的最小人脸尺寸，比如需要检测到30*30的人脸则设置为30，
// 该尺寸占图像比例越小则检测速度越慢，具体请参考性能指标章节。默认值30
void Setting::set_min_face_size(BaiduFaceApi *api)
{
    api->set_min_face_size(200);
}
// 跟踪到目标后执行检测的时间间隔，单位毫秒，默认300，值越小越会更快发现新目标，
// 但是cpu占用率会提高、处理速度变慢
void Setting::set_track_by_detection_interval(BaiduFaceApi *api)
{
    api->set_track_by_detection_interval(300);
}
// 未跟踪到目标时的检测间隔，单位毫秒，默认300，值越小越快发现新目标，
// 但是cpu占用率会提高、处理速度变慢
void Setting::set_detect_in_video_interval(BaiduFaceApi *api)
{
    api->set_detect_in_video_interval(300);
}
