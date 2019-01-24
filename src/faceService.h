#ifndef INCLUDE_FACE_SERVICE_H
#define INCLUDE_FACE_SERVICE_H
#include <map>
#include <memory>

#include <string>
#include <vector>
#include "baidu_face_api.h"
#include <pthread.h>
#include <unistd.h>
#include <mutex>
#include "predis/redis_pool.h"
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

struct FaceUpdateResult {
  std::string faceToken;
  Location location;
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
  FeatureBuffer(std::shared_ptr<RedisPool> redisPool) {
    redisPool_ = redisPool;
  }
  
  bool setRedisPool(std::shared_ptr<RedisPool> redisPool) {
    redisPool_ = redisPool;
  }
  std::shared_ptr<FaceBuffer> getBuffer(const std::string &faceToken);
  void addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer);

 private:
  int getBufferIndex(); 
  std::map<std::string, std::shared_ptr<FaceBuffer> > faceBuffers[2];
  std::shared_ptr<RedisPool> redisPool_{nullptr}; 
  std::mutex lock_;
};

class FaceService {
 public:
  static FaceService& getFaceService();
  FaceService();
  /* init baiduapi(now only support one instance), facelib*/
  int init(std::shared_ptr<RedisPool> pool);
  /* detect face, caculate feature and buffer it with facetoken*/
  int detect(const std::vector<unsigned char> &data, 
             int faceNum,
             std::vector<FaceDetectResult> &result);
  /*compare the feature with prestored facelib*/
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

  int updateUserFace(const std::string &groupId,
                            const std::string &userId,
                            const std::string &userName,
                            const std::string &dataBase64,
                            FaceUpdateResult &updateResult);

  int match(const std::string &faceToken, 
              const std::string &faceTokenCompare,
              float &score);
              
  int matchImage(const std::string &image64,                   
                     const std::string &image64Compare,              
                     float &score);
                     
  int matchImageToken(const std::string &image64,                   
                      const std::string &faceToken,              
                      float &score);

  void setFeatureBuffer(std::shared_ptr<FeatureBuffer>         featureBuffer) {
    featureBuffers_ = featureBuffer;
  }

 private:
  int search(std::shared_ptr<BaiduFaceApi> api,
      const std::set<std::string> &groupIds, 
      const std::vector<float> &feature,
      int num,
      std::vector<FaceSearchResult> &result);
      
  std::shared_ptr<FaceAttr> getAttr(const unsigned char *data, int len);
  
  std::shared_ptr<FaceAttr> getAttr(const unsigned char *data, 
                                       int len, 
                                       std::shared_ptr<BaiduFaceApi> api);
                                       
  std::shared_ptr<FaceQuality> faceQuality(const unsigned char *data, int len);
  
  std::shared_ptr<FaceQuality> faceQuality(const unsigned char *data, 
                                                int len,
                                                std::shared_ptr<BaiduFaceApi> api);
  /*load facelib*/
  int initAgent();
  /* baiduapi buffer*/                                             
  BaiduFaceApiBuffer apiBuffers_;
  /*face feature buffer ordered by faceToken, clear by day*/
  std::shared_ptr<FeatureBuffer>  featureBuffers_{nullptr}; 
  /*facelib lock*/
  pthread_rwlock_t faceLock_; 
};
}

#endif
