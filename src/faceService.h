#ifndef INCLUDE_FACE_SERVICE_H
#define INCLUDE_FACE_SERVICE_H
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "baidu_face_api.h"
namespace kface {
struct FaceBuffer {
  std::vector<float> feature;
};

struct Location {
  int x;
  int y;
  int width;
};

struct FaceAttr {
  int age;
  int gender;
  double genderConfidence;
};

struct FaceDetectResult {
  std::string faceToken;
  std::shared_ptr<FaceAttr> attr;
  Location location;
  TrackFaceInfo trackInfo;
};

struct FaceSearchResult {
  std::string userId;
  std::string groupId;
  std::string userName;
  double score;
};

class FaceService {
 public:
  static FaceService& getFaceService();
  FaceService();
  int init();
  std::shared_ptr<FaceAttr> getAttr(const unsigned char *data, int len);
  int detect(const std::vector<unsigned char> &data, 
             int faceNum,
             std::vector<FaceDetectResult> &result);
  int search(const std::set<std::string> &groupIds, 
      const std::string &faceToken,
      int num,
      std::vector<FaceSearchResult> &result);

  int addUserFace(const std::string &groupId,
                  const std::string &userId,
                  const std::string &userName,
                  const std::string &dataBase64,
                  std::string &faceToken);

  int delUserFace(const std::string &groupId,
                  const std::string &userId,
                  const std::string &faceToken);

 private:
  int initAgent(); 
  std::map<std::string, FaceBuffer> faceBuffers;
  std::shared_ptr<BaiduFaceApi>  api_{nullptr};
};
}

#endif
