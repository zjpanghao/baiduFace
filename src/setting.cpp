
#include "setting.h"

/**
*  �������Ϊ�������������ã������ü����С������С�����ù��ա�ģ���ȵȲ���
*  �û���ͨ���޸ĸ���Ĳ���ֵ�����ﵽ����������⡢ʶ�������Ŀ�ġ�
*/
Setting::Setting()
{
}

Setting::~Setting()
{
}
// �����Ƿ�ִ��������⣬Ĭ��Ϊfalse
void Setting::set_is_check_quality(BaiduFaceApi *api)
{
    api->set_isCheckQuality(true);
}
// ������ֵ��ȡֵ��Χ0~255��Ĭ��40��Խ���������Խ��
void Setting::set_illum_thr(BaiduFaceApi *api)
{
    api->set_illum_thr(40);
}
// ģ����ֵ��ȡֵ��Χ0~1��Ĭ��0.7��ԽС����ͼ��Խ����
// ������Ϊ1ʱ��ģ����ⲻ�����ҽ����ͨ����������Ϊ0ʱ��ģ����ⲻ�����ҽ��ͨ��
void Setting::set_blur_thr(BaiduFaceApi *api)
{
    api->set_blur_thr(0.7);
}

// ���ۡ����ۡ����ӡ���͡������ա������ա��°͵��ڵ���ֵ��ȡֵ��Χ0~1��Ĭ��0.5��ֵԽС�����ڵ�����ԽС
// ������Ϊ1ʱ���ڵ���ⲻ�����ҽ����ͨ����������Ϊ0ʱ���ڵ���ⲻ�����ҽ��ͨ��
void Setting::set_occlu_thr(BaiduFaceApi *api)
{
    api->set_occlu_thr(0.5);
}
// ����pitch��yaw��roll�ȽǶȵ���ֵ��Ĭ�϶�Ϊ15&deg;��Խ�����ɲɼ��������Ƕ�Խ��
// ���ǽǶ�Խ��ʶ��Ч����Խ��
void Setting::set_eulur_angle_thr(BaiduFaceApi *api)
{
    api->set_eulur_angle_thr(15, 15, 15);
}
// �����������Ŷ���ֵ��ȡֵ��Χ0~1��ȡ0����Ϊ���м������Ľ������������Ĭ��0.5
void Setting::set_notface_thr(BaiduFaceApi *api)
{
    api->set_notFace_thr(0.5);
}
// ��С�����ߴ磺��Ҫ��⵽����С�����ߴ磬������Ҫ��⵽30*30������������Ϊ30��
// �óߴ�ռͼ�����ԽС�����ٶ�Խ����������ο�����ָ���½ڡ�Ĭ��ֵ30
void Setting::set_min_face_size(BaiduFaceApi *api)
{
    api->set_min_face_size(200);
}
// ���ٵ�Ŀ���ִ�м���ʱ��������λ���룬Ĭ��300��ֵԽСԽ����췢����Ŀ�꣬
// ����cpuռ���ʻ���ߡ������ٶȱ���
void Setting::set_track_by_detection_interval(BaiduFaceApi *api)
{
    api->set_track_by_detection_interval(300);
}
// δ���ٵ�Ŀ��ʱ�ļ��������λ���룬Ĭ��300��ֵԽСԽ�췢����Ŀ�꣬
// ����cpuռ���ʻ���ߡ������ٶȱ���
void Setting::set_detect_in_video_interval(BaiduFaceApi *api)
{
    api->set_detect_in_video_interval(300);
}
