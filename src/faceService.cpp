#include "faceService.h"
#include "image_base64.h"
#include "md5.h"
#include "faceAgent.h"
#include "faceRepo.h"
#include <glog/logging.h>
#include <regex>
#include <iterator>
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

  initAgent();
  return 0;
}

/*
 * only detect one person
 */
int FaceService::detect(const std::vector<unsigned char> &data,
    int faceNum,
    std::vector<FaceDetectResult> &detectResult 
    ) {
  int rc = 0;
  Mat m = imdecode(Mat(data), 1);
#if 0
  int count = api_->get_face_feature_by_buf(&data[0], data.size(), feature);
  if (count != 512) {
    return -1;
  }
#endif

  std::unique_ptr< std::vector<TrackFaceInfo> >  out(new std::vector<TrackFaceInfo>());
  std::vector<TrackFaceInfo> *vec = out.get();
  int nFace = api_->track(vec, m, faceNum);
  if (nFace <= 0) {
    return -2;
  }
  LOG(INFO) << "track size:" << api_->get_faces()->size();
  for (TrackFaceInfo &info : *out) {
    FaceDetectResult result;
    result.trackInfo = info;
    int x = info.box.mCenter_x - info.box.mWidth / 2;
    int y = info.box.mCenter_y - info.box.mWidth / 2;
    result.location.x = x;
    result.location.y = y;
    LOG(INFO) << "CX" << info.box.mCenter_x;
    LOG(INFO) << "CY" << info.box.mCenter_y;
    LOG(INFO) << "The x is " << x;
    LOG(INFO) << "The y is " << y;
    LOG(INFO) << "The width is " << info.box.mWidth;

    result.location.width= info.box.mWidth;
    Mat child(m, Rect(x, y, result.location.width, result.location.width)); 
    const float *feature = nullptr;
    int count = api_->get_face_feature(child, feature);
    printf("Feature : %x\n", feature);
    if (count != 512) {
      continue;
    }
    std::vector<unsigned char> childImage;
    imencode(".bmp", child, childImage);  
    result.attr = getAttr(&childImage[0], childImage.size());
    result.faceToken = MD5(ImageBase64::encode(&childImage[0], childImage.size())).toStr();
    FaceBuffer buffer;
    buffer.feature.assign(feature, feature + 512);
    faceBuffers.insert(std::make_pair(result.faceToken, buffer)); 
    detectResult.push_back(result);
    api_->clearTrackedFaces();
  } 
  
  return rc;
}

int FaceService::addUserFace(const std::string &groupId,
    const std::string &userId,
    const std::string &userName,
    const std::string &dataBase64,
    std::string &faceToken){
  FaceAgent &faceAgent = FaceAgent::getFaceAgent();
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
  auto it = faceBuffers.find(result.faceToken);
  if (it == faceBuffers.end()) {
    return -3;
  }
  face.feature = it->second.feature;
  face.faceToken = result.faceToken;
  int rc = faceAgent.addPersonFace(face);
  if (rc == 0) {
    flushFaces();
  }
  faceToken = face.faceToken;
  return rc;
}

int FaceService::search(const std::set<std::string> &groupIds, 
                        const std::string &faceToken, 
                        int num,
                        std::vector<FaceSearchResult> &searchResult) {
  for (auto &v : groupIds) {
    printf("%s\n", v.c_str());
  }
  auto it = faceBuffers.find(faceToken);
  if (it == faceBuffers.end()) {
    return -1;
  }

  std::vector<PersonFace> top;
  FaceBuffer &faceBuffer = it->second;
  FaceAgent &faceAgent = FaceAgent::getFaceAgent();
  std::list<PersonFace> faces;
  faceAgent.getDefaultPersonFaces(faces);
  PersonFace *target = NULL;
  float maxScore = 0;
  std::map<std::string, float> userScore;
  for (auto &face : faces) {
    if (groupIds.count(face.groupId) == 0) {
      continue;
    }
    float score = api_->compare_feature(face.feature, faceBuffer.feature);
    std::string key = face.groupId + "_" +face.userId; 
    if (userScore[key] < score)  {
      userScore[key] = score;
    }
  }

  LOG(INFO) << "userScore" <<  userScore.size(); 
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
      searchResult.push_back(tmp);
    }
  }
  return 0;
}

std::shared_ptr<FaceAttr>  FaceService::getAttr(const unsigned char *data, int len){
  std::shared_ptr<FaceAttr> attr;
  std::string result =  api_->face_attr_by_buf(data, len);
  Json::Value root;
  Json::Reader reader;;
  std::stringstream ss;
  if (true == reader.parse(result, root)) {
    attr.reset(new FaceAttr());
    ss << root["data"]["result"]["age"].asString(); 
    ss >> attr->age;
    ss.clear();
    ss.str("");
    ss <<  root["data"]["result"]["gender"].asString(); 
    ss >> attr->gender;
    ss.clear();
    ss.str("");
    ss <<  root["data"]["result"]["gender_conf"].asString(); 
    ss >> attr->genderConfidence;
  }
  return attr;
}

int FaceService::delUserFace(const std::string &groupId,
    const std::string &userId,
    const std::string &faceToken) {
  int rc = 0;


}

}
