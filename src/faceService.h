#ifndef INCLUDE_FACE_SERVICE_H
#define INCLUDE_FACE_SERVICE_H
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "baidu_face_api.h"
#include <pthread.h>
#include <unistd.h>
#include <mutex>
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

class BaiduFaceApiBuffer {
 public:
  BaiduFaceApiBuffer() {
  }
  std::shared_ptr<BaiduFaceApi> borrowBufferedApi();
  
  void offerBufferedApi(std::shared_ptr<BaiduFaceApi> api); 

  int init(int bufferNums);

 private:
  std::shared_ptr<BaiduFaceApi> getInitApi(); 
  std::list<std::shared_ptr<BaiduFaceApi>> apis_;
  std::mutex lock_;
};

class BaiduApiWrapper {
 public:
   explicit BaiduApiWrapper(BaiduFaceApiBuffer &buffers)
    : buffers_(buffers) {
      api_ = buffers.borrowBufferedApi();
   }
   ~BaiduApiWrapper() {
     if (api_ != nullptr) {
       buffers_.offerBufferedApi(api_);
     }
   }

   std::shared_ptr<BaiduFaceApi> getApi() {
     int count = 0;
     while (api_ == nullptr) {
       api_ = buffers_.borrowBufferedApi();
       sleep(1);
       count++;
       if (count > 3) {
         break;
       }
     }
     return api_;
   }

 private:
   std::shared_ptr<BaiduFaceApi> api_{nullptr};
   BaiduFaceApiBuffer &buffers_;
};

class FeatureBuffer {
 public:
  std::shared_ptr<FaceBuffer> getBuffer(const std::string &faceToken);
  void addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer);

 private:
  int getBufferIndex(); 
  std::map<std::string, std::shared_ptr<FaceBuffer> > faceBuffers[2];
  std::mutex lock_;
};

class FaceService {
 public:
  static FaceService& getFaceService();
  FaceService();
  int init();
  std::shared_ptr<FaceAttr> getAttr(const unsigned char *data, int len);
  std::shared_ptr<FaceAttr> getAttr(const unsigned char *data, 
      int len, 
      std::shared_ptr<BaiduFaceApi> api);
  std::shared_ptr<FaceQuality> faceQuality(const unsigned char *data, int len);
  std::shared_ptr<FaceQuality> faceQuality(const unsigned char *data, 
      int len,
      std::shared_ptr<BaiduFaceApi> api);
  int detect(const std::vector<unsigned char> &data, 
             int faceNum,
             std::vector<FaceDetectResult> &result);
  int search(const std::set<std::string> &groupIds, 
      const std::string &faceToken,
      int num,
      std::vector<FaceSearchResult> &result);
  int searchByImage64(const std::set<std::string> &groupIds, 
      const std::string &imageBase64,
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
  int search(std::shared_ptr<BaiduFaceApi> api,
      const std::set<std::string> &groupIds, 
      const std::vector<float> &feature,
      int num,
      std::vector<FaceSearchResult> &result);
  BaiduFaceApiBuffer apiBuffers_;
  FeatureBuffer  featureBuffers_;
  int initAgent(); 
  //std::shared_ptr<BaiduFaceApi>  api_{nullptr};
  pthread_rwlock_t faceLock_; 
};
}

#endif
