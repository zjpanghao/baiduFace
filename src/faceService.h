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
  int height;
  float rotation;
};

struct FaceAttr {
  int age;
  int gender;
  double genderConfidence;
  int expression;
  int glasses;
};

struct Occlution {
  double leftEye;
  double rightEye;
  double nose;
  double mouth;
  double leftCheek;
  double rightCheek;
  double chinContour;
};

struct FaceQuality {
  int illumination;
  double blur;
  int completeness;
  Occlution occlution;
};

struct FaceDetectResult {
  std::string faceToken;
  std::shared_ptr<FaceAttr> attr;
  std::shared_ptr<FaceQuality> quality;
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
  std::shared_ptr<FaceQuality> faceQuality(const unsigned char *data, int len);
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

  int delUser(const std::string &groupId,
              const std::string &userId);

  int delUserFace(const std::string &groupId,
                  const std::string &userId,
                  const std::string &faceToken);

 private:
  int getBufferIndex();
  int initAgent(); 
  void addBuffer(std::string &faceToken, FaceBuffer &buffer);
  std::map<std::string, FaceBuffer> faceBuffers[2];
  std::shared_ptr<BaiduFaceApi>  api_{nullptr};
};
}

#endif
