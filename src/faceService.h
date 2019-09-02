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
#include "db/dbpool.h"
#include "faceRepoSql.h"
#include <unordered_map>
#include "apipool/apiPool.h"
namespace kface {
class FeatureBuffer;
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

class BaiduFaceApiService {
  public:
    int init() {
      int rc = api_.sdk_init(false);
      if (rc != 0) {
        return -1;
      }
      if (!api_.is_auth()) {
        return -1;
      }
      api_.set_min_face_size(15);
      return 0;
    }

    BaiduFaceApi* api() {
      return &api_;
    }
    
  private:
    BaiduFaceApi api_;
};

class FaceService {
 public:
  static FaceService& getFaceService();
  FaceService();
  int init(std::shared_ptr<DBPool> pool, 
      std::shared_ptr<FeatureBuffer> featureBuffer);
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

  std::shared_ptr<FaceAttr> getAttr(const unsigned char *data, int len);

  std::shared_ptr<DBPoolInfo> getPoolInfo() {
    std::shared_ptr<DBPoolInfo> info = std::make_shared<DBPoolInfo> ();
    faceLibRepo_->getPool()->PoolSizeGet(info->size);
    faceLibRepo_->getPool()->PoolActiveSizeGet(info->activeSize);
    return info;
  }

 private:
  int search(std::shared_ptr<BaiduFaceApiService> api,
      const std::set<std::string> &groupIds, 
      const std::vector<float> &feature,
      int num,
      std::vector<FaceSearchResult> &result);
  
  std::shared_ptr<FaceAttr> getAttr(const unsigned char *data, 
                                       int len, 
                                       std::shared_ptr<BaiduFaceApiService> api);
                                       
  std::shared_ptr<FaceQuality> faceQuality(const unsigned char *data, int len);
  
  std::shared_ptr<FaceQuality> faceQuality(const unsigned char *data, 
                                                int len,
                                                std::shared_ptr<BaiduFaceApiService> api);
  /*load facelib*/
  int initAgent();
  /*face feature buffer ordered by faceToken, clear by day*/
  std::shared_ptr<FeatureBuffer>  featureBuffers_{nullptr}; 

  std::shared_ptr<FaceLibRepo> faceLibRepo_;
  /*baiduapi*/
  ApiBuffer<BaiduFaceApiService> apiBuffers_;
};

}

#endif
