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
#include "glog/logging.h"
#include "faceAgent.h"
#include "personFaceRepo.h"
#include "faceService.h"
using namespace kface;
int test_detect(const char *fname) {
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(fname, out_buf);
  std::vector<unsigned char> data(&out_buf[0], &out_buf[0] + buf_len);
  LOG(INFO) << buf_len << "data:" << data.size();
  FaceService service = FaceService::getFaceService();
  LOG(INFO) << service.init();
  std::vector<FaceDetectResult> result;
  int rc = service.detect(data, 1, result);
  LOG(INFO) << rc << "size:" << result.size();
}

int main() {
  test_detect("1.jpg");
  return 0;
}
