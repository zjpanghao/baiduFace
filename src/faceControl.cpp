#include "faceControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif
#include "event2/http_compat.h"
#include "json/json.h"
#include "image_base64.h"
#include "event2/http.h"

#include<opencv2/opencv.hpp>
#include<opencv/highgui.h>

#include <thread>
#include <vector>
#include <glog/logging.h>
#include <iterator>
#include <regex>
#include "faceAgent.h"
#include "faceService.h"
#include "util.h"
#include "httpUtil.h"

namespace kface {
void faceDetectCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  std::vector<FaceDetectResult> result;
  Json::Value root;
  Json::Value faceResult;
  Json::Value items;
 
  evbuffer *response = evbuffer_new();
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    rc = -1;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  
  if (!getBodyJson(req, root)) {
    rc = -3;
    sendResponse(rc, "parse error", req, response);
    return;
  }
  std::string data;
  JsonUtil::getJsonStringValue(root, "image", data);
  if (data.empty()) {
    rc = -4;
    sendResponse(rc, "image error", req, response);
    return;
  }
  int faceNum = 1;
  JsonUtil::getJsonValue(root, "max_face_num", faceNum);
  LOG(INFO) << "max_face_num:" << faceNum << "image:" << data.length();
  std::string decodeData;
  int decodeLen = 0;
  decodeData = ImageBase64::decode(data.c_str(), data.size(), decodeLen);
  std::vector<unsigned char> cdata;
  cdata.assign(&decodeData[0], &decodeData[0] + decodeLen);
  FaceService &service = FaceService::getFaceService(); 
  rc = service.detect(cdata, faceNum, result);
  if (rc != 0) {
    rc = -4;
    result.clear();
    sendResponse(rc, "detect error", req, response);
    return;
  }

  if (rc == 0 && result.size() == 0) {
    sendResponse(222202, "pic not has face", req, response);
    return;
  }
  
  faceResult["error_code"] = "0";
  faceResult["error_msg"] = "SUCCESS";
  Json::Value content;
  auto it = result.begin();
  while (it != result.end()) {
    Json::Value item;
    item["face_token"] = it->faceToken;
    Json::Value faceType;
    faceType["probability"] = 1;
    faceType["type"] = "human";
    item["face_type"] = faceType;
    if (it->attr != nullptr) {
      Json::Value gender;
      gender["type"] = it->attr->gender == 1 ? "male" : "female";
      gender["probability"] = it->attr->genderConfidence;
      item["gender"] = gender;
      item["age"] = it->attr->age;
      Json::Value glasses;
      glasses["type"] = it->attr->glasses ? "WITH" : "NONE";
      item["glasses"] = glasses;
      Json::Value expression;
      expression["type"] = it->attr->expression ? "smile" : "none";
      item["expression"] = expression;
    } else {
      Json::Value gender;
      gender["type"] = "male";
      gender["probability"] = 0.99;
      item["gender"] = gender;
      item["age"] = 30;
      Json::Value glasses;
      glasses["type"] = "NONE";
      item["glasses"] = glasses;
      Json::Value expression;
      expression["type"] = "none";
      item["expression"] = expression;
    }
    
    item["face_probability"] =  it->trackInfo.score;
    Json::Value location;
    location["left"] = it->location.x;
    location["top"] = it->location.y;
    location["width"] = it->location.width;
    location["height"] = it->location.height;
    location["rotation"] = it->location.rotation;
    item["location"] = location;
    if (it->quality != nullptr) {
      Json::Value quality;
      quality["illumination"] = it->quality->illumination;
      quality["blur"] = it->quality->blur;
      quality["completeness"] = it->quality->completeness;
      Json::Value occl;
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
      Json::Value quality;
      quality["illumination"] = 100;
      quality["blur"] = 0;
      quality["completeness"] = 1;
      Json::Value occl;
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

    Json::Value headPose;
    int inx = 0;
    std::vector<std::string> angleNames{"roll", "pitch", "yaw"};
    for (float v : it->trackInfo.headPose) {
      headPose[angleNames[inx++]] = v;
    }
    item["angle"] = headPose;
    items.append(item);
    it++;
  }
  content["face_num"] = (int)result.size();
  content["face_list"] = items;
  faceResult["result"] = content;
  if (result.size() > 0) {
    LOG(INFO) << faceResult.toStyledString();
  }
  evbuffer_add_printf(response, "%s", faceResult.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

void faceIdentifyCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  Json::Value root;
  int decodeLen = 0;
  auto start = std::chrono::steady_clock::now();
  evbuffer *response = evbuffer_new();
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    rc = -1;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  if (!getBodyJson(req, root)) {
    rc = -3;
    sendResponse(rc, "parse error", req, response);
    return;
  }
  std::string faceData;
  JsonUtil::getJsonStringValue(root, "image", faceData);
  std::string imageType;
  JsonUtil::getJsonStringValue(root, "image_type", imageType);
  int faceNum;
  JsonUtil::getJsonValue(root, "max_user_num", faceNum);
  std::string groupIds;
  JsonUtil::getJsonStringValue(root, "group_id_list", groupIds);
  if (faceData.empty() || imageType.empty() || groupIds.empty()) {
    rc = -4;
    sendResponse(rc, "params error", req, response);
    return;
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
    sendResponse(rc, "search error", req, response);
    return;
  }
  Json::Value faceResult;
  faceResult["error_code"] = "0";
  Json::Value content;
  Json::Value items;
  for (FaceSearchResult &result : resultList) {
    Json::Value item;
    item["user_id"] = result.userId;
    item["score"] = result.score;
    item["user_info"] = result.userName;
    item["group_id"] = result.groupId;
    items.append(item);
    content["user_list"] = items;
  }
  faceResult["result"] = content;
 
  if (resultList.size() > 0) {
    faceResult["error_msg"] = "SUCCESS";
  } else {
    Json::Value content;
    faceResult["error_code"] = "222207";
    faceResult["error_msg"] = "match user is not found";
    faceResult["result"] = content;
  }
  
  auto end = std::chrono::steady_clock::now();
  auto dureTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
  LOG(INFO) << "dure:" << dureTime;
  //faceResult["dure"] = dureTime;
  LOG(INFO) << faceResult.toStyledString();
  evbuffer_add_printf(response, "%s", faceResult.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

void faceDebugCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  Json::Value root;
  evbuffer *response = evbuffer_new();
  FaceService &service = FaceService::getFaceService(); 
  auto poolInfo = service.getPoolInfo();
  Json::Value faceResult;
  faceResult["error_code"] = "0";
  Json::Value content;
  Json::Value item;
  if (poolInfo != nullptr) {
    item["size"] = poolInfo->size;
    item["score"] = poolInfo->activeSize;
  }
  content["pool_info"] = item;
  faceResult["result"] = content;
  faceResult["error_msg"] = "SUCCESS";
  LOG(INFO) << faceResult.toStyledString();
  evbuffer_add_printf(response, "%s", faceResult.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

static void faceMatchCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  Json::Value root;
  int decodeLen = 0;
  evbuffer *response = evbuffer_new();
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    rc = -1;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  if (!getBodyJson(req, root) || !root.isArray() || root.size() != 2) {
    rc = -3;
    sendResponse(rc, "parse error", req, response);
    return;
  }
  std::string faceData[2];
  std::string imageType[2];
  for (int i = 0; i < 2; i++) {
    JsonUtil::getJsonStringValue(root[i], "image", faceData[i]);
    JsonUtil::getJsonStringValue(root[i], "image_type", imageType[i]);
    if (faceData[i].empty() || imageType[i].empty()) {
      rc = -4;
      sendResponse(rc, "params error", req, response);
      return;
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
    sendResponse(rc, "match error", req, response);
    return;
  }
  Json::Value faceResult;
  Json::Value compareResult;
  faceResult["error_code"] = "0";
  faceResult["error_msg"] = "SUCCESS";
  compareResult["score"] = score;
  faceResult["result"] = compareResult;
 
  LOG(INFO) << faceResult.toStyledString();
  evbuffer_add_printf(response, "%s", faceResult.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

void initFaceControl(std::vector<HttpControl> &controls) {
  std::vector<HttpControl> controlList = {
    {"/face-api/v3/face/detect", faceDetectCb},
    {"/face-api/v3/face/identify", faceIdentifyCb},
    {"/face-api/v3/face/debug", faceDebugCb},
    {"/face-api/v3/face/match", faceMatchCb}
  };
  for (HttpControl &control : controlList) {
    controls.push_back(control);
  }
}

}

