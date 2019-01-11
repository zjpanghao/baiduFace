#include "faceService.h"
#include "image_base64.h"
#include "md5.h"
#include "faceAgent.h"
#include "faceRepo.h"
#include <glog/logging.h>
#include <regex>
#include <iterator>
#include "cv_help.h"
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
  api_->clearTrackedFaces();
  Mat m = imdecode(Mat(data), 1);
#if 0 
  std::vector<TrackFaceInfo>   *out = new std::vector<TrackFaceInfo> ();
  int nFace = api_->track(out, m, faceNum);
  const float *feature = nullptr;

  for (TrackFaceInfo &info : *out) {
    FaceDetectResult result;
    result.trackInfo = info;
    RotatedRect rRect = CvHelp::bounding_box(info.landmarks);
    Rect rect = rRect.boundingRect();
    LOG(INFO) << "x:" << rect.x << "y:" << rect.y << "width" << rect.width << "height" << rect.height;
    Mat child(m, Rect(rect.x, rect.y, rect.width, rect.height)); 
    result.faceToken = MD5(ImageBase64::encode(&data[0], data.size())).toStr();
    std::vector<unsigned char> childImage;
    imencode(".jpg", child, childImage);  
    int count = api_->get_face_feature_by_buf(&childImage[0],childImage.size(), feature);
    printf("Feature : %x\n", feature);
    if (count != 512) {
      return -1;
    }

    FaceBuffer buffer;
    buffer.feature.assign(feature, feature + 512);
    int inx = getBufferIndex();
    int old = 1 - inx;
    if (!faceBuffers[old].empty()) {
      LOG(INFO) <<"clear buffer size:" <<  faceBuffers[old].size();
      faceBuffers[old].clear();
    }
    faceBuffers[inx].insert(std::make_pair(result.faceToken, buffer)); 
    detectResult.push_back(result);
  }

#else

  std::unique_ptr<std::vector<TrackFaceInfo>>   out(new std::vector<TrackFaceInfo> ());
  std::vector<TrackFaceInfo> *vec = out.get();
  int nFace = api_->track(vec, m, faceNum);
  if (nFace <= 0) {
    return -2;
  }
  LOG(INFO) << "track size:" << api_->get_faces()->size();
  for (TrackFaceInfo &info : *out) {
    FaceDetectResult result;
    result.trackInfo = info;
    RotatedRect rRect = CvHelp::bounding_box(info.landmarks);
    Rect rect = rRect.boundingRect();
    int x = rect.x;
    int y = rect.y;
    if (x < 0 || y < 0) { return -3;};
    if (rect.width + x > m.cols) {
      return -3;
    }
    if (rect.height + y > m.rows) {
      return -3;
    }
    result.location.x = x;
    result.location.y = y;

    result.location.width= rect.width;
    result.location.height= rect.height;
    result.location.rotation = rRect.angle;
    
    Mat child(m, Rect(x, y, result.location.width, result.location.height)); 
    LOG(INFO) << "child" << child.isContinuous();
    std::vector<unsigned char> childImage;
    imencode(".jpg", child, childImage);  
    const float *feature = nullptr;
    int count = api_->get_face_feature_by_buf(&childImage[0], childImage.size(), feature);
    printf("Feature : %x\n", feature);
    if (count != 512) {
      continue;
    }
    FaceBuffer buffer;
    buffer.feature.assign(feature, feature + 512);
    result.attr = getAttr(&childImage[0], childImage.size());
    result.quality = faceQuality(&childImage[0], childImage.size());
    result.faceToken = MD5(ImageBase64::encode(&childImage[0], childImage.size())).toStr();
    if (true) {
      std::string name = "image/" + result.faceToken + ".jpg";
      imwrite(name.c_str(), child);
    }
    int inx = getBufferIndex();
    int old = 1 - inx;
    if (!faceBuffers[old].empty()) {
      LOG(INFO) <<"clear buffer size:" <<  faceBuffers[old].size();
      faceBuffers[old].clear();
    }
    faceBuffers[inx].insert(std::make_pair(result.faceToken, buffer)); 
    detectResult.push_back(result);
  } 
#endif
  api_->clearTrackedFaces();
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
  int inx = getBufferIndex();
  auto it = faceBuffers[inx].find(result.faceToken);
  if (it == faceBuffers[inx].end()) {
    return -3;
  }
  face.image.reset(new ImageFace());
  face.image->feature= it->second.feature;
  face.image->faceToken = result.faceToken;
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
  for (auto &v : groupIds) {
    printf("%s\n", v.c_str());
  }
  int inx = getBufferIndex();
  auto it = faceBuffers[inx].find(faceToken);
  if (it == faceBuffers[inx].end()) {
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
    float score = api_->compare_feature(face.image->feature, faceBuffer.feature);
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
      searchResult.push_back(tmp);
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
  std::string result =  api_->face_attr_by_buf(data, len);
  LOG(INFO) << result;
  Json::Value root;
  Json::Reader reader;;
  std::stringstream ss;
  if (true == reader.parse(result, root)) {
    if (!root["errno"].isNull() && root["errno"].asInt() == 0) {
      attr.reset(new FaceAttr());
      getBaiString(root, "age", attr->age);
      getBaiString(root, "gender", attr->gender);
      getBaiString(root, "gender_conf", attr->genderConfidence);
      LOG(INFO) << "GEN C" << attr->genderConfidence;
      getBaiString(root, "glass", attr->glasses);
      getBaiString(root, "expression", attr->expression);
    }
  }
  return attr;
}

std::shared_ptr<FaceQuality>  FaceService::faceQuality(const unsigned char *data, int len){
  std::shared_ptr<FaceQuality> value;
  std::string result =  api_->face_quality_by_buf(data, len);
  LOG(INFO) << result;
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
  FaceAgent &agent = FaceAgent::getFaceAgent();
  rc = agent.delPerson(face);
  if (rc == 0) {
    flushFaces();
  }
  return rc;
}

int FaceService::getBufferIndex() {
  time_t current = time(NULL);
  struct tm val;
  localtime_r(&current, &val);
  int inx = val.tm_mday % 2;
  return inx;
}

}
