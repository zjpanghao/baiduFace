#include "faceService.h"
#include <sys/time.h>
#include <set>
#include "image_base64.h"
#include "md5.h"
#include "faceAgent.h"
#include "faceRepoSql.h"
#include <glog/logging.h>
#include <regex>
#include <iterator>
#include "cv_help.h"
#include "util.h"
#include "predis/redis_pool.h"
#include "featureBufferMemory.h"
#define BAIDU_FEATURE_KEY "baiduFeature"
#define MAX_FACE_TRACK 5
using  cv::Mat;
using cv::RotatedRect;
using cv::Rect;

namespace kface {
FaceService& FaceService::getFaceService() {
  static FaceService faceService;
  return faceService;
}

FaceService::FaceService() {
}

int FaceService::initAgent() {
  return 0;
}

int FaceService::init(std::shared_ptr<DBPool> pool, 
                        std::shared_ptr<FeatureBuffer> featureBuffer) {
  apiBuffers_.init(1);
  faceLibRepo_ = std::make_shared<FaceLibRepo>(pool);
  featureBuffers_ = featureBuffer;
  featureBuffers_->init();
  return 0;
}

int FaceService::detect(const std::vector<unsigned char> &data,
    int faceNum,
    std::vector<FaceDetectResult> &detectResult) {
  int rc = 0;
  ApiWrapper<BaiduFaceApiService> baidu(apiBuffers_);
  auto baiduApi = baidu.getApi();
  if (baiduApi == nullptr || data.size() < 10) {
    return -1;
  }
  auto api = baiduApi->api();
  api->clearTrackedFaces();
  Mat m = imdecode(Mat(data), 1);
  std::unique_ptr<std::vector<TrackFaceInfo>> out(new std::vector<TrackFaceInfo>());
  std::vector<TrackFaceInfo> *vec = out.get();
  int nFace = api->track(vec, m, faceNum);
  if (nFace <= 0) {
    return -2;
  }
  for (TrackFaceInfo &info : *out) {
    FaceDetectResult result;
    result.trackInfo = info;
    /*calculate faceRect*/
    RotatedRect rRect = CvHelp::bounding_box(info.landmarks);
    Rect rect = rRect.boundingRect();
    int x = rect.x;
    int y = rect.y;
    if (x < 0) {x = 0;}
    if (y < 0) {y = 0;}
    if (rect.width + x > m.cols) {
      rect.width = m.cols - x - 1;
    }

    if (rect.height + y > m.rows) {
      rect.height = m.rows - y -1;
    }
    if (rect.width < 0 || rect.height < 0) { 
      LOG(ERROR) << "Error face Rect detect (x)" << x <<"(y) "<< y;
      return -3;
    }
    result.location.x = x;
    result.location.y = y;
    result.location.width= rect.width;
    result.location.height= rect.height;
    result.location.rotation = rRect.angle;
    
    /*getfeature and save*/
    Mat child(m, Rect(x, y, result.location.width, result.location.height)); 
    std::vector<unsigned char> childImage;
    imencode(".jpg", child, childImage);  
    const float *feature = nullptr;
    int count = api->get_face_feature_by_buf(&childImage[0], childImage.size(), feature);
    if (count != 512) {
      continue;
    }
  
    std::shared_ptr<FaceBuffer> buffer(new FaceBuffer());
    buffer->feature.assign(feature, feature + 512);
    result.attr = nullptr;
    result.quality = nullptr;
    result.faceToken = MD5(ImageBase64::encode(&childImage[0], childImage.size())).toStr(); 
    featureBuffers_->addBuffer(result.faceToken, buffer);
    detectResult.push_back(result);
  } 

  api->clearTrackedFaces();
  return rc;
}

int FaceService::search(const std::set<std::string> &groupIds, 
                        const std::string &faceToken, 
                        int num,
                        std::vector<FaceSearchResult> &searchResult) {

  std::shared_ptr<FaceBuffer> featureBuffer = featureBuffers_->getBuffer(faceToken);
  if (featureBuffer == nullptr) {
    return -1;
  }
  ApiWrapper<BaiduFaceApiService> baidu(apiBuffers_);
  auto baiduApi = baidu.getApi();
  return search(baiduApi, groupIds, featureBuffer->feature, num, searchResult);
}

int FaceService::match(const std::string &faceToken,                   
    const std::string &faceTokenCompare,              
    float &score) {
  std::shared_ptr<FaceBuffer> featureBuffer = featureBuffers_->getBuffer(faceToken);
  std::shared_ptr<FaceBuffer> featureBufferCompare = featureBuffers_->getBuffer(faceTokenCompare);
  if (featureBuffer == nullptr || featureBufferCompare == nullptr) {
    return -1;
  }
  ApiWrapper<BaiduFaceApiService> baidu(apiBuffers_);
  auto baiduApi = baidu.getApi();
  auto api = baiduApi->api();
  score = api->compare_feature(featureBuffer->feature, featureBufferCompare->feature);
  return 0;
}

 int FaceService::matchImageToken(const std::string &image64,                   
                                         const std::string &faceToken,              
                                         float &score) {
   ApiWrapper<BaiduFaceApiService> baidu(apiBuffers_);
   auto baiduApi = baidu.getApi();
   auto api = baiduApi->api();
   const float *feature = NULL;
   int count = api->get_face_feature(image64.c_str(), 1, feature);
   if (count != 512) {
     return -2;
   }
   std::vector<float> data;
   data.assign(feature, feature + 512);
   std::shared_ptr<FaceBuffer> featureBuffer = featureBuffers_->getBuffer(faceToken);
   if (featureBuffer == nullptr) {
     return -1;
   }
   score = api->compare_feature(featureBuffer->feature, data);
   return 0;
 }

int FaceService::matchImage(const std::string &image64,                   
                                 const std::string &image64Compare,              
                                 float &score) {
  ApiWrapper<BaiduFaceApiService> baidu(apiBuffers_);
  auto baiduApi = baidu.getApi();
  auto api = baiduApi->api();
  const float *feature = NULL;
  int count = api->get_face_feature(image64.c_str(), 1, feature);
  if (count != 512) {
   return -2;
  }
  std::vector<float> data;
  data.assign(feature, feature + 512);

  count = api->get_face_feature(image64Compare.c_str(), 1, feature);
  if (count != 512) {
   return -2;
  }
  std::vector<float> dataCompare;
  dataCompare.assign(feature, feature + 512); 
  score = api->compare_feature(dataCompare, data);
  return 0;
}

int FaceService::searchByImage64(const std::set<std::string> &groupIds, 
    const std::string &imageBase64, 
    int num,
    std::vector<FaceSearchResult> &searchResult) {
  int len = 0;
  std::string data = ImageBase64::decode(imageBase64.c_str(), imageBase64.length(), len);
  if (len < 10) {
    return -1;
  }
  std::vector<unsigned char> vdata((unsigned char*)&data[0], (unsigned char*)&data[len]);
  std::vector<FaceDetectResult> detectResult;
  int rc = detect(vdata, 1, detectResult);
  if (rc < 0 || detectResult.size() != 1) {
    LOG(ERROR) << "detect face error" << "rc:" << rc;
    return -1;
  }
  return search(groupIds, detectResult[0].faceToken, 1, searchResult);
}
            
int FaceService::search(std::shared_ptr<BaiduFaceApiService> baiduApi,
    const std::set<std::string> &groupIds, 
    const std::vector<float> &feature,
    int num,
    std::vector<FaceSearchResult> &result) {
  auto api = baiduApi->api();
  std::vector<PersonFace> top;
  std::list<PersonFace> faces;
  faceLibRepo_->loadPersonFaces("", faces);
  LOG(INFO) << "load faces size:" << faces.size();
  if (faces.empty()) {
    return 0;
  }
  /*get the highest mark of each userId*/
  std::map<std::string, float> userScore;
  auto &similarFace = faces.front();
  int maxScore = -1;
  for (auto &face : faces) {
    if (groupIds.count(face.groupId) == 0) {
      continue;
    }
    float score = api->compare_feature(face.image->feature, feature);
    if (num == 1) {
      if (score > maxScore) {
        maxScore = score;
        similarFace = face;
      }
    } else {
      std::string key = face.groupId + "_" +face.userId; 
      if (userScore[key] < score)  {
        userScore[key] = score;
      }
    }
  }

  if (num == 1) {
    if (maxScore > 0) {
      FaceSearchResult tmp;
      tmp.groupId = similarFace.groupId;
      tmp.userId = similarFace.userId;
      tmp.score = maxScore;
      tmp.userName = similarFace.userName;
      result.push_back(tmp);
    }
    return 0;
  }

  /* find the top num marks*/
  std::vector<std::pair<std::string,float>>  topPair(userScore.begin(), userScore.end());
  std::partial_sort(topPair.begin(), 
      num > topPair.size() ? topPair.end() : topPair.begin() + num, 
      topPair.end(),
      [](const std::pair<std::string, float> &a, const std::pair<std::string, float>  &b) {
      return a.second > b.second;});
  for (auto &p : topPair) {
    if (--num < 0) {
      return 0;
    }
    FaceSearchResult tmp;
    std::string key = p.first;
    std::regex re{"_"};
    std::vector<std::string> groupUser(std::sregex_token_iterator(key.begin(), key.end(), re, -1),
                                       std::sregex_token_iterator());
    if (groupUser.size() == 2) {
      tmp.groupId = groupUser[0];
      tmp.userId = groupUser[1];
      tmp.score = p.second;
      result.push_back(tmp);
    }
  }
  return 0;
}

template<class E>
static void getBaiString(Json::Value &value, const std::string &key, E &t) {
  Json::Value &tmp = value["data"]["result"];
  if (tmp.isNull() || tmp[key].isNull()) {
    return;
  }
  std::stringstream ss;
  ss << tmp[key].asString();
  ss >> t;
}

std::shared_ptr<FaceAttr>  FaceService::getAttr(const unsigned char *data, int len){
  std::shared_ptr<FaceAttr> attr;
  ApiWrapper<BaiduFaceApiService> baidu(apiBuffers_);
  auto baiduApi = baidu.getApi();
  return getAttr(data, len, baiduApi);
}

std::shared_ptr<FaceAttr>  FaceService::getAttr(const unsigned char *data, 
    int len,
    std::shared_ptr<BaiduFaceApiService> baiduApi){
  if (baiduApi == nullptr) {
    return nullptr;
  }
  auto api = baiduApi->api();
  std::shared_ptr<FaceAttr> attr;
  std::string result =  api->face_attr_by_buf(data, len);
  LOG(INFO) << result;
  Json::Value root;
  Json::Reader reader;;
  if (true == reader.parse(result, root)) {
    if (!root["errno"].isNull() && root["errno"].asInt() == 0) {
      attr.reset(new FaceAttr());
      getBaiString(root, "age", attr->age);
      getBaiString(root, "gender", attr->gender);
      getBaiString(root, "gender_conf", attr->genderConfidence);
      getBaiString(root, "glass", attr->glasses);
      getBaiString(root, "expression", attr->expression);
    }
  }
  return attr;
}

std::shared_ptr<FaceQuality>  FaceService::faceQuality(const unsigned char *data, int len){
  std::shared_ptr<FaceQuality> value;
  ApiWrapper<BaiduFaceApiService> baidu(apiBuffers_);
  auto baiduApi = baidu.getApi();
  return faceQuality(data, len, baiduApi);
}

std::shared_ptr<FaceQuality>  FaceService::faceQuality(const unsigned char *data, int len,
    std::shared_ptr<BaiduFaceApiService> baiduApi){
  auto api = baiduApi->api();
  std::shared_ptr<FaceQuality> value;
  std::string result =  api->face_quality_by_buf(data, len);
  Json::Value root;
  Json::Reader reader;;
  std::stringstream ss;
  if (true == reader.parse(result, root)) {
    if (!root["errno"].isNull() && root["errno"].asInt() == 0) {
      value.reset(new FaceQuality());
      value->completeness = 1;
      getBaiString(root, "bluriness", value->blur);
      getBaiString(root, "illum", value->illumination);
      getBaiString(root, "occl_l_eye", value->occlution.leftEye);
      getBaiString(root, "occl_r_eye", value->occlution.rightEye);
      getBaiString(root, "occl_l_contour", value->occlution.leftCheek);
      getBaiString(root, "occl_r_contour", value->occlution.rightCheek);;
      getBaiString(root, "occl_mouth", value->occlution.mouth);
      getBaiString(root, "occl_nose", value->occlution.nose);
      getBaiString(root, "occl_chin",value->occlution.chinContour);
    }
  }
  return value;
}

int FaceService::addUserFace(const std::string &groupId,
    const std::string &userId,
    const std::string &userName,
    const std::string &dataBase64,
    std::string &faceToken){
  int len = 0;
  std::string data = ImageBase64::decode(dataBase64.c_str(), dataBase64.length(), len);
  if (len < 10) {
    return -1;
  }
  PersonFace face;
  face.appName = DEFAULT_APP_NAME;
  face.groupId = groupId;
  face.userId = userId;
  face.userName = userName;
  const float *feature = nullptr;
  std::vector<unsigned char> mdata(&data[0], &data[0] + len);
  std::vector<FaceDetectResult> results;
  if (0 != detect(mdata, 1, results)) {
    return -1;
  }
  if (results.size() != 1) {
    return -2;
  }
  
  FaceDetectResult &result = results[0];
  std::shared_ptr<FaceBuffer> featureBuffer = featureBuffers_->getBuffer(result.faceToken);
  if (featureBuffer == nullptr) {
    return -3;
  }
  face.image.reset(new ImageFace());
  face.image->feature= featureBuffer->feature;
  face.image->faceToken = result.faceToken;
  if (0 != faceLibRepo_->addUserFace(face)) {
    LOG(ERROR) << "repo add userface error";
    return -9;
  }
  return 0;
}

int FaceService::updateUserFace(const std::string &groupId,
    const std::string &userId,
    const std::string &userName,
    const std::string &dataBase64,
    FaceUpdateResult &updateResult) {
  int len = 0;
  return -1;
}

int FaceService::delUserFace(const std::string &groupId,
    const std::string &userId,
    const std::string &faceToken) {
  int rc = 0;
  PersonFace face;
  face.appName = DEFAULT_APP_NAME;
  face.groupId = groupId;
  face.userId = userId;
  face.image.reset(new ImageFace());
  face.image->faceToken = faceToken;
  if ((rc = faceLibRepo_->delUserFace(face)) < 0) {
    LOG(ERROR) << "repo del face error" << rc;
    return -5;
  }
  LOG(INFO) << "del face token:" << faceToken << "userId" << userId <<  "rc:" << rc;
  return rc;
}

int FaceService::delUser(const std::string &groupId,
    const std::string &userId) {
  int rc = 0;
  PersonFace face;
  face.appName = DEFAULT_APP_NAME;
  face.groupId = groupId;
  face.userId = userId;
  rc = faceLibRepo_->delUser(face);
  if (rc < 0) {
    LOG(ERROR) << "repo del user:" << rc;
    return -1;
  }
  LOG(INFO) << "del user  userId" << userId <<  "rc:" << rc;
  return rc;
}

}
