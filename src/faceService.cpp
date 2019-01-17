#include "faceService.h"
#include "image_base64.h"
#include "md5.h"
#include "faceAgent.h"
#include "faceRepo.h"
#include <glog/logging.h>
#include <regex>
#include <iterator>
#include "cv_help.h"
#include "util.h"
#define MAX_FACE_TRACK 5
using namespace cv;
namespace kface {
FaceService& FaceService::getFaceService() {
  static FaceService faceService;
  return faceService;
}

FaceService::FaceService() {
}

int FaceService::initAgent() {
  pthread_rwlock_init(&faceLock_, NULL);
  FaceAgent &faceAgent = FaceAgent::getFaceAgent();
  std::list<PersonFace> faces;
  loadPersonFaces(faces);
  LOG(INFO) << "load persons :" << faces.size();
  for (PersonFace &face : faces) {
    faceAgent.addPersonFace(face);
  }
  return 0;
}

int FaceService::init() {
#if 0
  if (api_ != nullptr) {
    return 0;
  }
  LOG(INFO) << "faceService init";
  api_.reset(new BaiduFaceApi());
  int rc = api_->sdk_init(false);
  if (rc != 0) {
    return rc;
  }
  if (!api_->is_auth()) {
    return -1;
  }
  api_->set_min_face_size(15);
#endif
  apiBuffers_.init(1);
  initAgent();
  return 0;
}

/*
 * only detect one person
 */
int FaceService::detect(const std::vector<unsigned char> &data,
    int faceNum,
    std::vector<FaceDetectResult> &detectResult) {
  int rc = 0;
  BaiduApiWrapper baidu(apiBuffers_);
  auto api = baidu.getApi();
  if (api == nullptr) {
    return -1;
  }
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
    RotatedRect rRect = CvHelp::bounding_box(info.landmarks);
    Rect rect = rRect.boundingRect();
    int x = rect.x;
    int y = rect.y;
    if (x < 0 || y < 0 || rect.width + x > m.cols || rect.height + y > m.rows) { 
		LOG(ERROR) << "Error face Rect detect (x)" << x <<"(y) "<< y;
		return -3;
	};
   
    result.location.x = x;
    result.location.y = y;

    result.location.width= rect.width;
    result.location.height= rect.height;
    result.location.rotation = rRect.angle;
    
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
    result.attr = getAttr(&childImage[0], childImage.size(), api);
    result.quality = faceQuality(&childImage[0], childImage.size(), api);
    result.faceToken = MD5(ImageBase64::encode(&childImage[0], childImage.size())).toStr(); 
	  featureBuffers_.addBuffer(result.faceToken, buffer);
    detectResult.push_back(result);
	/*
	** 记录图片信息
	*/
    //{
    //  std::string name = "image/" + result.faceToken + ".jpg";
    //  imwrite(name.c_str(), child);
    //}
  } 

  api->clearTrackedFaces();
  return rc;
}

int FaceService::addUserFace(const std::string &groupId,
    const std::string &userId,
    const std::string &userName,
    const std::string &dataBase64,
    std::string &faceToken){
  int len = 0;
  std::string data = ImageBase64::decode(dataBase64.c_str(), dataBase64.length(), len);
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
  std::shared_ptr<FaceBuffer> featureBuffer = featureBuffers_.getBuffer(result.faceToken);
  if (featureBuffer == nullptr) {
    return -3;
  }
  face.image.reset(new ImageFace());
  face.image->feature= featureBuffer->feature;
  face.image->faceToken = result.faceToken;
  WLockMethod wlock;
  RWLockGuard guard(wlock, &faceLock_);
  FaceAgent &faceAgent = FaceAgent::getFaceAgent();
  int rc = faceAgent.addPersonFace(face);
  if (rc == 0) {
    flushFaces();
  }
  faceToken = face.image->faceToken;
  LOG(INFO) << "add face token:" << faceToken << "rc:" << rc;
  return rc;
}

int FaceService::search(const std::set<std::string> &groupIds, 
                        const std::string &faceToken, 
                        int num,
                        std::vector<FaceSearchResult> &searchResult) {

  std::shared_ptr<FaceBuffer> featureBuffer = featureBuffers_.getBuffer(faceToken);
  if (featureBuffer == nullptr) {
    return -1;
  }
  BaiduApiWrapper baidu(apiBuffers_);
  auto api = baidu.getApi();
  if (api == nullptr) {
    return -2;
  }
  return search(api, groupIds, featureBuffer->feature, num, searchResult);
}

int FaceService::searchByImage64(const std::set<std::string> &groupIds, 
                        const std::string &imageBase64, 
                        int num,
                        std::vector<FaceSearchResult> &searchResult) {

  BaiduApiWrapper baidu(apiBuffers_);
  auto api = baidu.getApi();
  if (api == nullptr) {
    return -1;
  }
  const float *feature = nullptr;
  int count = api->get_face_feature(imageBase64.c_str(), 1, feature);
  if (count != 512) {
    return -2;
  }
  std::vector<float> data;
  data.assign(feature, feature + 512);
  return search(api, groupIds, data, num, searchResult);
}
						
int FaceService::search(std::shared_ptr<BaiduFaceApi> api,
    const std::set<std::string> &groupIds, 
    const std::vector<float> &feature,
    int num,
    std::vector<FaceSearchResult> &result) {

  std::vector<PersonFace> top;
  RLockMethod rlock;
  RWLockGuard guard(rlock, &faceLock_);
  FaceAgent &faceAgent = FaceAgent::getFaceAgent();
  std::list<PersonFace> faces;
  faceAgent.getDefaultPersonFaces(faces);

  std::map<std::string, float> userScore;
  for (auto &face : faces) {
    if (groupIds.count(face.groupId) == 0) {
      continue;
    }
    float score = api->compare_feature(face.image->feature, feature);
    std::string key = face.groupId + "_" +face.userId; 
    if (userScore[key] < score)  {
      userScore[key] = score;
    }
  }

  std::vector<std::pair<std::string,float>>  topPair(userScore.begin(), userScore.end());
  std::partial_sort(topPair.begin(), num > topPair.size() ? topPair.end() : topPair.begin() + num, topPair.end(),
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
  BaiduApiWrapper wapper(apiBuffers_);
  auto api = wapper.getApi();
  return getAttr(data, len, api);
}

std::shared_ptr<FaceAttr>  FaceService::getAttr(const unsigned char *data, 
    int len,
    std::shared_ptr<BaiduFaceApi> api){
  if (api == nullptr) {
    return nullptr;
  }
  std::shared_ptr<FaceAttr> attr;
  std::string result =  api->face_attr_by_buf(data, len);
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
  BaiduApiWrapper wapper(apiBuffers_);
  auto api = wapper.getApi();
  if (api == nullptr) {
    return nullptr;
  }
  return faceQuality(data, len, api);
}

std::shared_ptr<FaceQuality>  FaceService::faceQuality(const unsigned char *data, int len,
    std::shared_ptr<BaiduFaceApi> api){
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
  WLockMethod wlock;
  RWLockGuard guard(wlock, &faceLock_);
  FaceAgent &agent = FaceAgent::getFaceAgent();
  rc = agent.delPersonFace(face);
  if (rc == 0) {
    flushFaces();
  }
  return rc;
}

int FaceService::delUser(const std::string &groupId,
    const std::string &userId) {
  int rc = 0;
  PersonFace face;
  face.appName = DEFAULT_APP_NAME;
  face.groupId = groupId;
  face.userId = userId;
  WLockMethod wlock;
  RWLockGuard guard(wlock, &faceLock_);
  FaceAgent &agent = FaceAgent::getFaceAgent();
  rc = agent.delPerson(face);
  if (rc == 0) {
    flushFaces();
  }
  return rc;
}

int FeatureBuffer::getBufferIndex() {
  time_t current = time(NULL);
  struct tm val;
  localtime_r(&current, &val);
  int inx = val.tm_mday % 2;
  return inx;
}

std::shared_ptr<FaceBuffer> FeatureBuffer::getBuffer(const std::string &faceToken) {
  int inx = getBufferIndex();
  std::lock_guard<std::mutex> guard(lock_);
  auto it = faceBuffers[inx].find(faceToken);
  if (it == faceBuffers[inx].end()) {
    return nullptr;
  }
  return it->second;
}

void FeatureBuffer::addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer) {
  int inx = getBufferIndex();
  int old = 1 - inx;
  std::lock_guard<std::mutex> guard(lock_);
  if (!faceBuffers[old].empty()) {
    LOG(INFO) <<"clear buffer size:" <<  faceBuffers[old].size();
    faceBuffers[old].clear();
  }
  faceBuffers[inx].insert(std::make_pair(faceToken, buffer));
}

int BaiduFaceApiBuffer::init(int bufferNums) {
  for (int i = 0; i < bufferNums; i++) {
    auto api = getInitApi();
    if (api != nullptr) {
      apis_.push_back(api);
    }
  }
  return apis_.empty() ? -1 : 0; 
}

std::shared_ptr<BaiduFaceApi> BaiduFaceApiBuffer::getInitApi() {
  std::shared_ptr<BaiduFaceApi> api(new BaiduFaceApi());
  int rc = api->sdk_init(false);
  if (rc != 0) {
    return nullptr;
  }
  if (!api->is_auth()) {
    return nullptr;
  }
  return api;
}

std::shared_ptr<BaiduFaceApi> BaiduFaceApiBuffer::borrowBufferedApi() {
  std::lock_guard<std::mutex> guard(lock_);
  if (apis_.empty()) {
    return nullptr;
  }
  auto api = apis_.front();
  apis_.pop_front();
  return api;
}

void BaiduFaceApiBuffer::offerBufferedApi(std::shared_ptr<BaiduFaceApi> api) {
  if (api == nullptr) {
    return;
  }
  std::lock_guard<std::mutex> guard(lock_);
  apis_.push_back(api);
}

}
