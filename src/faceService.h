#ifndef INCLUDE_FACE_SERVICE_H
#define INCLUDE_FACE_SERVICE_H
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "baidu_face_api.h"
namespace kface {
struct FaceBuffer {
  std::vector<float> feature;
};

struct FaceDetectResult{
  std::string faceToken;
  TrackFaceInfo trackInfo;
};

class FaceService {
 public:
  static FaceService& getFaceService();
  FaceService();
  int init();
  int detect(const std::vector<unsigned char> &data, 
             int faceNum,
             std::vector<FaceDetectResult> &result);
 private:
  int initAgent(); 
  std::map<std::string, FaceBuffer> faceBuffers;
  std::shared_ptr<BaiduFaceApi>  api_{nullptr};
};
}

#endif
