#include "faceControl.h"

#include "json/json.h"
#include "image_base64.h"
#include <thread>
#include <vector>
#include <glog/logging.h>
#include <iterator>
#include <regex>
#include "faceService.h"
#include "jsonUtil.h"
#include "util.h"
#include "httpUtil.h"
namespace kface {
int  FaceControl::faceIdentifyCb(
    const pson::Json::Value &root, 
    pson::Json::Value &result) {
  int rc = 0;
  int decodeLen = 0;
  auto start = std::chrono::steady_clock::now();
  std::string faceData;
  JsonUtil::getJsonString(root, "image", faceData);
  std::string imageType;
  JsonUtil::getJsonString(root, "image_type", imageType);
  int faceNum = 1;
  baidu::JsonUtil::getJsonValue(root, "max_user_num", faceNum);
  std::string groupIds;
  JsonUtil::getJsonString(root, "group_id_list", groupIds);
  if (faceData.empty() || imageType.empty() || groupIds.empty()) {
    rc = -4;
    setResponse(rc, "params error", result);
    LOG(ERROR) << result.toStyledString();
    return rc;
  }
  std::regex re(",");
  std::set<std::string> groupList(std::sregex_token_iterator(groupIds.begin(), groupIds.end(), re, -1),
      std::sregex_token_iterator());
  LOG(INFO) << "faceToken:" << (faceData.length() < 64 ? faceData: faceData.substr(0,64))  << "gid:" << groupIds << "type" << imageType;

  std::vector<FaceSearchResult> resultList;
  FaceService &service = FaceService::getFaceService(); 
  if (imageType == "FACE_TOKEN") {
    rc = service.search(groupList, faceData, faceNum, resultList);
  } else if (imageType == "BASE64") {
    rc = service.searchByImage64(groupList, faceData, faceNum, resultList);
  } else {
    rc = -5;
  }

  if (rc != 0) {
    rc = -4;
    setResponse(rc, "search error", result);
    LOG(ERROR) << result.toStyledString();
    return rc;
  }
  pson::Json::Value &faceResult = result;
 
  faceResult["error_code"] = "0";
  pson::Json::Value content;
  pson::Json::Value items;
  for (FaceSearchResult &faceSearchResult : resultList) {
    pson::Json::Value item;
    item["user_id"] = 
      faceSearchResult.userId;
    item["score"] 
      = faceSearchResult.score;
    item["user_info"] 
      = faceSearchResult.userName;
    item["group_id"] 
      = faceSearchResult.groupId;
    items.append(item);
    content["user_list"] = items;
  }
  faceResult["result"] = content;
 
  if (resultList.size() > 0) {
    faceResult["error_msg"] = "SUCCESS";
  } else {
    pson::Json::Value content;
    faceResult["error_code"] = "222207";
    faceResult["error_msg"] = "match user is not found";
    faceResult["result"] = content;
  }
  
  auto end = std::chrono::steady_clock::now();
  auto dureTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
  LOG(INFO) << faceResult.toStyledString();
  LOG(INFO) << "dure:" << dureTime;
  return 0;
  //faceResult["dure"] = dureTime;
}
int  FaceControl::faceDetectCb(
    const pson::Json::Value &root, 
    pson::Json::Value &result) {
  auto start = std::chrono::steady_clock::now();
  std::vector<FaceDetectResult> 
    detectResult;
  std::string data;
  JsonUtil::getJsonString(root, "image", data);
  int rc = 0;
  if (data.empty()) {
    rc = -1;
    setResponse(rc, "image error", result);
    LOG(ERROR) << result.toStyledString();
    return rc;
  }
  int faceNum = 1;
  baidu::JsonUtil::getJsonValue(root, "max_face_num", faceNum);
  LOG(INFO) << "max_face_num:" << faceNum << "image:" << data.length();
  std::string decodeData;
  int decodeLen = 0;
  decodeData = ImageBase64::decode(data.c_str(), data.size(), decodeLen);
  std::vector<unsigned char> cdata;
  cdata.assign(&decodeData[0], &decodeData[0] + decodeLen);
  FaceService &service = FaceService::getFaceService(); 
  rc = service.detect(cdata, faceNum, detectResult);
  if (rc != 0) {
    rc = -4;
    result.clear();
    setResponse(rc, "detect error", 
        result);
    LOG(ERROR) << result.toStyledString();
    return rc;
  }

  auto  &faceResult = result;
  if (rc == 0 && detectResult.size() == 0) {
    setResponse(222202, "pic not has face", result);
    return rc;
  }
  faceResult["error_code"] = "0";
  faceResult["error_msg"] = "SUCCESS";
  pson::Json::Value content;
  pson::Json::Value item;
  pson::Json::Value items;
  auto it = detectResult.begin();
  while (it != detectResult.end()) {
    pson::Json::Value item;
    item["face_token"] = it->faceToken;
    pson::Json::Value faceType;
    faceType["probability"] = 1;
    faceType["type"] = "human";
    item["face_type"] = faceType;
    if (it->attr != nullptr) {
      pson::Json::Value gender;
      gender["type"] = it->attr->gender == 1 ? "male" : "female";
      gender["probability"] = it->attr->genderConfidence;
      item["gender"] = gender;
      item["age"] = it->attr->age;
      pson::Json::Value glasses;
      glasses["type"] = "NONE";
      item["glasses"] = glasses;
      pson::Json::Value expression;
      expression["type"] = it->attr->expression ? "smile" : "none";
      item["expression"] = expression;
    } else {
      pson::Json::Value gender;
      gender["type"] = "male";
      gender["probability"] = 0.99;
      item["gender"] = gender;
      item["age"] = 30;
      pson::Json::Value glasses;
      glasses["type"] = "NONE";
      item["glasses"] = glasses;
      pson::Json::Value expression;
      expression["type"] = "none";
      item["expression"] = expression;
    }
    
    item["face_probability"] =  it->trackInfo.score;
    pson::Json::Value location;
    location["left"] = it->location.x;
    location["top"] = it->location.y;
    location["width"] = it->location.width;
    location["height"] = it->location.height;
    location["rotation"] = it->location.rotation;
    item["location"] = location;
    if (it->quality != nullptr) {
      pson::Json::Value quality;
      quality["illumination"] = it->quality->illumination;
      quality["blur"] = it->quality->blur;
      quality["completeness"] = it->quality->completeness;
      pson::Json::Value occl;
      occl["left_eye"] = (int)it->quality->occlution.leftEye;
      occl["right_eye"] = (int)it->quality->occlution.rightEye;
      occl["left_cheek"] = (int)it->quality->occlution.leftCheek;
      occl["right_cheek"] = (int)it->quality->occlution.rightCheek;
      occl["mouth"] = (int)it->quality->occlution.mouth;
      occl["nose"] = (float)it->quality->occlution.nose;
      occl["chin_contour"] = (int)it->quality->occlution.chinContour;
      quality["occlusion"] = occl;
      quality["completeness"] = it->quality->completeness;
      item["quality"] = quality;
    } else {
      pson::Json::Value quality;
      quality["illumination"] = 100;
      quality["blur"] = 0;
      quality["completeness"] = 1;
      pson::Json::Value occl;
      occl["left_eye"] = 0;
      occl["right_eye"] = 0;
      occl["left_cheek"] = 0;
      occl["right_cheek"] = 0;
      occl["mouth"] = 0;
      occl["nose"] = 0.0;
      occl["chin_contour"] = 0;
      quality["occlusion"] = occl;
      item["quality"] = quality;
    }

    pson::Json::Value headPose;
    int inx = 0;
    std::vector<std::string> angleNames{"roll", "pitch", "yaw"};
    for (float v : it->trackInfo.headPose) {
      headPose[angleNames[inx++]] = v;
    }
    item["angle"] = headPose;
    items.append(item);
    it++;
  }
  content["face_num"] = (int)detectResult.size();
  content["face_list"] = items;
  faceResult["result"] = content;
  auto end = std::chrono::steady_clock::now();
  auto dureTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
  LOG(INFO) << faceResult.toStyledString();
  LOG(INFO) << "dure:" << dureTime;
  return 0;
}

std::vector<HttpControl>
  FaceControl::getMapping() {
    std::vector<HttpControl> controlList = {
      {"/face-api/v3/face/detect", FaceControl::faceDetectCb},
      {"/face-api/v3/face/identify", FaceControl::faceIdentifyCb},
      {"/face-api/v3/face/debug", FaceControl::faceDebugCb},
        {"/face-api/v3/face/match", FaceControl::faceMatchCb}
    };
    return controlList;
  }

int FaceControl::faceDebugCb(
    const pson::Json::Value &root, 
    pson::Json::Value &result) {
  int rc = 0;
  FaceService &service = FaceService::getFaceService(); 
  auto poolInfo = service.getPoolInfo();
  pson::Json::Value &faceResult = result;
  faceResult["error_code"] = "0";
  pson::Json::Value content;
  pson::Json::Value item;
  pson::Json::Value db;
  pson::Json::Value redis;
  if (poolInfo != nullptr) {
    db["size"] = poolInfo->size;
    db["active"] = poolInfo->activeSize;
  }
  content["db_info"] = db;
  auto redisInfo = service.getRedisPoolInfo();
  if (redisInfo != nullptr) {
    redis["size"] = redisInfo->size;
    redis["active"] = redisInfo->activeSize;
  }
  content["redis_info"] = redis;
  faceResult["result"] = content;
  faceResult["error_msg"] = "SUCCESS";
  LOG(INFO) << faceResult.toStyledString();
  return 0;
}

int FaceControl:: faceMatchCb(
    const pson::Json::Value &root, 
    pson::Json::Value &result) {
  int rc = 0;
  int decodeLen = 0;
  std::string faceData[2];
  std::string imageType[2];
  for (int i = 0; i < 2; i++) {
    JsonUtil::getJsonString(root[i], "image", faceData[i]);
    JsonUtil::getJsonString(root[i], "image_type", imageType[i]);
    if (faceData[i].empty() || imageType[i].empty()) {
      rc = -4;
      setResponse(rc, "params error", result);
      LOG(ERROR) << result.toStyledString();
      return rc;
    }
  }

  std::vector<FaceSearchResult> resultList;
  FaceService &service = FaceService::getFaceService(); 
  float score;
  if (imageType[0] == "FACE_TOKEN" && imageType[1] == "FACE_TOKEN") {
    rc = service.match(faceData[0], faceData[1], score);
  } else if (imageType[0] == "BASE64" && imageType[1] == "BASE64") {
    rc = service.matchImage(faceData[0], faceData[1], score);
  } else if (imageType[0] == "BASE64" && imageType[1] == "FACE_TOKEN") {
    rc = service.matchImageToken(faceData[0], faceData[1], score);
  } else if (imageType[1] == "BASE64" && imageType[0] == "FACE_TOKEN") {
    rc = service.matchImageToken(faceData[1], faceData[0], score);
  } else {
    rc = -9;
  }
  
  if (rc != 0) {
    rc = -4;
    setResponse(rc, "match error", result);
    LOG(ERROR) << result.toStyledString();
    return rc;
  }
  pson::Json::Value &faceResult = result;
  pson::Json::Value compareResult;
  faceResult["error_code"] = "0";
  faceResult["error_msg"] = "SUCCESS";
  compareResult["score"] = score;
  faceResult["result"] = compareResult;
  LOG(INFO) << faceResult.toStyledString();
  return 0;
}

int FaceControl::init(const kunyan::Config &config) {
  FaceService &service = 
    FaceService::getFaceService();
  return service.init(config);
}

}

