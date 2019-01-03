#include "image_buf.h"
#include <iostream>
	
const int ImageBuf::get_buf(const std::string &file_path, std::string &out)
{
    if (file_path.length() <= 0)
    {
        return 0;
    }
    int len = 0;
    FILE *fp = fopen(file_path.c_str(), "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        out.resize(len);
        fread((void*)out.data(), 1, len, fp);
        fclose(fp);
    }
    else
    {
        std::cout << "image file not exist" << std::endl;
    }
    return len;
}
