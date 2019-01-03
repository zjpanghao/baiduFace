#ifndef TESTFACEAPI_TESTFACEAPI_CV_HELP_H
#define TESTFACEAPI_TESTFACEAPI_CV_HELP_H
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "opencv2/opencv.hpp"
// 绘制类，绘制人脸框，人脸关键点等
class CvHelp
{
public:
    CvHelp();
    ~CvHelp();
public:
    static std::vector<std::string> argumens(int argc, char* argv[]);
    static std::string get_time();
    static std::vector<std::string> readlines(const std::string& filename);
    static void draw_rotated_box(cv::Mat& img, const cv::RotatedRect& box, const cv::Scalar color);
    static void draw_shape(std::vector<int> const& shape, cv::Mat& img, cv::Scalar color);
    static void draw_shape(int const* shape, int size, cv::Mat& img, cv::Scalar color);
    static void draw_shape(std::vector<cv::Point> const& shape, cv::Mat& img, cv::Scalar color);
    static void draw_shape(cv::Point const* shape, size_t size, cv::Mat& img, cv::Scalar color);
    static cv::RotatedRect bounding_box(int const* landmarks, int size);
    static cv::RotatedRect bounding_box(std::vector<int>& landmarks);
    static void get_point_set_center_scale_inline(cv::Mat_<float>& shape, cv::Point2f& center, float& scale);
    static void get_point_set_center_scale_ori_inline(cv::Mat_<float>& shape, int pt_id1, int pt_id2, cv::Point2f& center, float& scale, float& tilt_angle);
    static void get_point_set_center_scale_ori_inline(std::vector<cv::Point2f>& pts, cv::Point2f& center, float& scale, float& tilt_angle);
    static void landmarklist2pts_vec(std::vector<float>& shape, std::vector<cv::Point2f>& shapePts);
    template<typename T>
    static inline cv::Mat_<float> get_affine_matrix(T& srcCenter, T& dstCenter, float alpha, float scale);
    static inline cv::Mat_<float> inverse_matrix(cv::Mat_<float>& M);
    static void crop_face(cv::Mat& srcImg, cv::Mat& dstImg, std::vector<cv::Point2f>& shapePts, float box_enlarge_ratio);
    static cv::Mat get_bgr_img(unsigned char* img, int rows, int cols, int imgType);
    static cv::Mat from(std::vector<int> const& imgs, int size);
    static cv::Mat from(int const* imgs, int size, int width);
};

#endif