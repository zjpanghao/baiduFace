#ifndef TESTFACEAPI_CPP_IMAGE_BUF_H
#define TESTFACEAPI_CPP_IMAGE_BUF_H
#include <string>
// 对图片进行读取二进制buffer
class ImageBuf
{
    public:
    // 获取图片二进制buffer
    static const int get_buf(const std::string &file_path, std::string &out);   
};
#endif
