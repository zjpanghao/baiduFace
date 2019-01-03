#include "cv_help.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "opencv2/opencv.hpp"

CvHelp::CvHelp()
{
}

CvHelp::~CvHelp()
{
}

std::vector<std::string> CvHelp::argumens(int argc, char* argv[])
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }
    return args;
}
std::string CvHelp::get_time()
{
    time_t nowtime;
    nowtime = time(NULL);

    struct tm *local;
    local = localtime(&nowtime);

    char buf[80];
    strftime(buf, 80, "%Y%m%d%H%M%S", local);
    return buf;
}

std::vector<std::string> CvHelp::readlines(const std::string& filename)
{
    std::vector<std::string> lines;
    {
        std::fstream fs(filename.c_str(), std::ios::in);
        if (!fs)
        {
            std::cerr << "can not open file: " << filename << std::endl;
            return lines;
        }
        std::string line;
        while (getline(fs, line))
        {
            lines.push_back(line);
        }
        fs.close();
    }
    return lines;
}

void CvHelp::draw_rotated_box(cv::Mat& img, const cv::RotatedRect& box, const cv::Scalar color)
{
    cv::Point2f vertices[4];
    box.points(vertices);
    for (int j = 0; j < 4; j++)
    {
        line(img, vertices[j], vertices[(j + 1) % 4], color);
    }
}

void CvHelp::draw_shape(std::vector<int> const& shape, cv::Mat& img, cv::Scalar color)
{
    int npoint = shape.size() / 2;
    int radius = 2;
    for (int i = 0; i < npoint; ++i)
    {
        circle(img, cv::Point((int)(shape[i * 2]), (int)(shape[i * 2 + 1])),
            radius, color, -1, 8, 0);
    }

    if (npoint == 72)
    {
        const int ncomponents = 9;
        int comp1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        int comp2[] = { 13, 14, 15, 16, 17, 18, 19, 20, 13, 21 };
        int comp3[] = { 22, 23, 24, 25, 26, 27, 28, 29, 22 };
        int comp4[] = { 30, 31, 32, 33, 34, 35, 36, 37, 30, 38 };
        int comp5[] = { 39, 40, 41, 42, 43, 44, 45, 46, 39 };
        int comp6[] = { 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 47 };
        int comp7[] = { 51, 57, 52 };
        int comp8[] = { 58, 59, 60, 61, 62, 63, 64, 65, 58 };
        int comp9[] = { 58, 66, 67, 68, 62, 69, 70, 71, 58 };

        int* idx[] = { comp1, comp2, comp3, comp4, comp5, comp6, comp7, comp8, comp9 };
        int npoints[ncomponents] = { 13, 10, 9, 10, 9, 11, 3, 9, 9 };

        for (int i = 0; i < ncomponents; ++i)
        {
            for (int j = 0; j < npoints[i] - 1; ++j)
            {
                line(img, cv::Point((int)(shape[idx[i][j] * 2]), (int)(shape[idx[i][j] * 2 + 1])),
                    cv::Point((int)(shape[idx[i][j + 1] * 2]), (int)(shape[idx[i][j + 1] * 2 + 1])),
                    CV_RGB(0, 0, 255), 1, 8, 0);
            }
        }
    }
}
void CvHelp::draw_shape(int const* shape, int size, cv::Mat& img, cv::Scalar color)
{
    int npoint = size / 2;
    int radius = 2;
    for (int i = 0; i < npoint; ++i)
    {
        circle(img,
            cv::Point((int)(shape[i * 2]), (int)(shape[i * 2 + 1])),
            radius, color, -1, 8, 0);
    }

    if (npoint == 72)
    {
        const int nComponents = 9;
        int comp1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        int comp2[] = { 13, 14, 15, 16, 17, 18, 19, 20, 13, 21 };
        int comp3[] = { 22, 23, 24, 25, 26, 27, 28, 29, 22 };
        int comp4[] = { 30, 31, 32, 33, 34, 35, 36, 37, 30, 38 };
        int comp5[] = { 39, 40, 41, 42, 43, 44, 45, 46, 39 };
        int comp6[] = { 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 47 };
        int comp7[] = { 51, 57, 52 };
        int comp8[] = { 58, 59, 60, 61, 62, 63, 64, 65, 58 };
        int comp9[] = { 58, 66, 67, 68, 62, 69, 70, 71, 58 };

        int* idx[] = { comp1, comp2, comp3, comp4, comp5, comp6, comp7, comp8, comp9 };
        int npoints[nComponents] = { 13, 10, 9, 10, 9, 11, 3, 9, 9 };

        for (int i = 0; i < nComponents; ++i)
        {
            for (int j = 0; j < npoints[i] - 1; ++j)
            {
                line(img,
                    cv::Point((int)(shape[idx[i][j] * 2]), (int)(shape[idx[i][j] * 2 + 1])),
                    cv::Point((int)(shape[idx[i][j + 1] * 2]), (int)(shape[idx[i][j + 1] * 2 + 1])),
                    CV_RGB(0, 0, 255), 1, 8, 0);
            }
        }
    }
}

void CvHelp::draw_shape(std::vector<cv::Point> const& shape, cv::Mat& img, cv::Scalar color)
{
    for (size_t i = 0; i < shape.size(); ++i)
    {
        circle(img, shape[i], 2, color, -1, 8, 0);
    }
}
void CvHelp::draw_shape(cv::Point const* shape, size_t size, cv::Mat& img, cv::Scalar color)
{
    for (size_t i = 0; i < size; ++i)
    {
        circle(img, shape[i], 2, color, -1, 8, 0);
    }
}

cv::RotatedRect CvHelp::bounding_box(int const* landmarks, int size)
{
    int min_x = 1000000;
    int min_y = 1000000;
    int max_x = -1000000;
    int max_y = -1000000;
    for (int i = 0; i < size / 2; ++i)
    {
        min_x = std::min(min_x, landmarks[2 * i]);
        min_y = std::min(min_y, landmarks[2 * i + 1]);
        max_x = std::max(max_x, landmarks[2 * i]);
        max_y = std::max(max_y, landmarks[2 * i + 1]);
    }
    int width = ((max_x - min_x) + (max_y - min_y)) / 2;
    float angle = 0;
    cv::Point2f center = cv::Point2f((min_x + max_x) / 2, (min_y + max_y) / 2);
    return cv::RotatedRect(center, cv::Size2f(width, width), angle);
}
cv::RotatedRect CvHelp::bounding_box(std::vector<int>& landmarks)
{
    return bounding_box(landmarks.data(), landmarks.size());
}

/// crop face
void CvHelp::get_point_set_center_scale_inline(cv::Mat_<float>& shape,
    cv::Point2f& center, float& scale)
{
    if (shape.empty())
    {
        std::cerr << "the imput shape is empty!" << std::endl;
        return;
    }
    //'''Get the bounding box's center and scale of the given points set'''
    float min_x = 10000;
    float min_y = 10000;
    float max_x = 0;
    float max_y = 0;
    for (int i = 0; i < shape.cols / 2; ++i)
    {
        if (shape(0, i * 2) < 0 || shape(0, i * 2 + 1) < 0)
        {
            continue;
        }
        min_x = std::min(min_x, shape(0, i * 2));
        max_x = std::max(max_x, shape(0, i * 2));
        min_y = std::min(min_y, shape(0, i * 2 + 1));
        max_y = std::max(max_y, shape(0, i * 2 + 1));
    }
    center.x = 0.5f * (min_x + max_x);
    center.y = 0.5f * (min_y + max_y);
    scale = abs(max_x - min_x); //0.5 * (abs(max_x - min_x) + abs(max_y - min_y));
}
void CvHelp::get_point_set_center_scale_ori_inline(cv::Mat_<float>& shape, 
    int pt_id1, int pt_id2, cv::Point2f& center, float& scale, float& tilt_angle)
{
    //'''get the norm info of a set of points given two of the pts that def the oriention'''
    tilt_angle = atan2(shape(0, 2 * pt_id2 + 1) - shape(0, 2 * pt_id1 + 1), 
        shape(0, 2 * pt_id2) - shape(0, 2 * pt_id1));
    get_point_set_center_scale_inline(shape, center, scale);
}

void CvHelp::get_point_set_center_scale_ori_inline(std::vector<cv::Point2f>& pts,
    cv::Point2f& center, float& scale, float& tilt_angle)
{
    cv::Mat_<float> shape(1, pts.size() * 2);
    for (int i = 0; i < pts.size(); ++i)
    {
        shape(0, i << 1) = pts[i].x;
        shape(0, 1 + (i << 1)) = pts[i].y;
    }

    if (pts.size() == 72 || pts.size() == 76)
    {
        cv::Point2f p1((pts[47].x + pts[56].x) / 2, (pts[47].y + pts[56].y) / 2);
        //tilt_angle = -atan2(-(p1.x - pts[6].x),-(p1.y - pts[6].y));
        tilt_angle = atan2((p1.y - pts[6].y), (p1.x - pts[6].x)) + CV_PI / 2;
        get_point_set_center_scale_inline(shape, center, scale);
    }
    else if (pts.size() == 9)
    {
        tilt_angle = atan2(pts[4].y - pts[0].y, pts[4].x - pts[0].x);
        get_point_set_center_scale_inline(shape, center, scale);
    }
    else
    {
        std::cerr << "pts size:" << pts.size() << std::endl;
        std::cerr << "the method getPointSetCenterScaleOri not implemented \
            for pts num not equal to 72" << std::endl;
    }
}

// inline void Affine_Point(const Mat_<float> &M,T& srcPt, T &dstPt);
void CvHelp::landmarklist2pts_vec(std::vector<float>& shape, 
    std::vector<cv::Point2f>& shapePts)
{
    int n_point = shape.size() / 2;
    for (size_t i = 0; i < n_point; ++i)
    {
        shapePts.push_back(cv::Point2f(shape[i * 2], shape[i * 2 + 1]));
    }
}

template<typename T>
inline cv::Mat_<float> CvHelp::get_affine_matrix(T& srcCenter, T& dstCenter,
    float alpha, float scale)
{
    cv::Mat_<float> m(2, 3);

    m(0, 0) = scale*cos(alpha);
    m(0, 1) = scale*sin(alpha);
    m(1, 0) = -m(0, 1);
    m(1, 1) = m(0, 0);

    m(0, 2) = srcCenter.x - m(0, 0)*dstCenter.x - m(0, 1)*dstCenter.y;
    m(1, 2) = srcCenter.y - m(1, 0)*dstCenter.x - m(1, 1)*dstCenter.y;
    return m;
}
inline cv::Mat_<float> CvHelp::inverse_matrix(cv::Mat_<float>& m)
{
    float d = m(0, 0)*m(1, 1) - m(0, 1)*m(1, 0);
    d = d != 0 ? 1. / d : 0;

    cv::Mat_<float> inv_m(2, 3);

    inv_m(0, 0) = m(1, 1)*d;
    inv_m(0, 1) = m(0, 1)*(-d);
    inv_m(1, 0) = m(1, 0)*(-d);
    inv_m(1, 1) = m(0, 0)*d;

    inv_m(0, 2) = -inv_m(0, 0)*m(0, 2) - inv_m(0, 1)*m(1, 2);
    inv_m(1, 2) = -inv_m(1, 0)*m(0, 2) - inv_m(1, 1)*m(1, 2);
    return inv_m;
} 

void CvHelp::crop_face(cv::Mat& srcImg, cv::Mat& dstImg, 
    std::vector<cv::Point2f>& shapePts, float box_enlarge_ratio)
{
    const int template_size = 400;

    dstImg.create(template_size, template_size, CV_8UC3);
    // const float box_enlarge_ratio = 2.0f;
    std::vector<cv::Point2f> pts_of_center_face;
    pts_of_center_face = shapePts; // assume one face for an img

    cv::Point2f pts_center(srcImg.cols / 2, srcImg.rows / 2);
    float pts_scale = srcImg.cols;
    float pts_ori = 0.0f;

    get_point_set_center_scale_ori_inline(pts_of_center_face, pts_center,
        pts_scale, pts_ori);

    //crop img
    // SimilarityTrans trans;

    cv::Point2f dst_center(dstImg.cols / 2, dstImg.rows / 2);
    float s = (pts_scale * box_enlarge_ratio) / float(dstImg.cols);

    // trans.setTransMatrix(pts_center, dstCenter, -pts_ori, s);
    cv::Mat_<float> trans = get_affine_matrix(pts_center, dst_center, -pts_ori, s);
    cv::warpAffine(srcImg, dstImg, inverse_matrix(trans), dstImg.size(),
        cv::INTER_LINEAR);
}
cv::Mat CvHelp::get_bgr_img(unsigned char* img, int rows, int cols, int imgType)
{
    cv::Mat image; // in BGR format
    cv::Mat tempImg(rows, cols, CV_8UC4, img);

#ifdef __APPLE__
    cvtColor(tempImg, image, CV_RGBA2BGR);
#else
    cvtColor(tempImg, image, CV_BGRA2BGR);
#endif
    return image;
}
cv::Mat CvHelp::from(std::vector<int> const& imgs, int size)
{
    if (imgs.size() != size*size)
    {
        return cv::Mat();
    }
    return get_bgr_img((unsigned char*)imgs.data(), size, size, 1);
}
cv::Mat CvHelp::from(int const* imgs, int size, int width)
{
    if (imgs == 0 || size != width*width)
    {
        return cv::Mat();
    }
    return get_bgr_img((unsigned char*)imgs, width, width, 1);
}