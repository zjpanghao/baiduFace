#include "userControl.h"
#include "json/json.h"
#include "image_base64.h"

#include <thread>
#include <vector>
#include <glog/logging.h>
#include <iterator>
#include <regex>
#include "faceService.h"
#include "httpUtil.h"
#include "util.h"

namespace kface {
/* delete user face */
int UserControl::userFaceDelCb(
    const Json::Value &root,
    Json::Value &result) {
  int rc = 0;
  int len = 0;
  Json::Value &delResult = result;
  FaceService &service = FaceService::getFaceService(); 
  std::string groupId;
  std::string userId;
  std::string faceToken;
  JsonUtil::JsonUtil::getJsonStringValue(root, "group_id", groupId);
  JsonUtil::getJsonStringValue(root, "user_id", userId);
  JsonUtil::getJsonStringValue(root, "face_token", faceToken);
  if (groupId.empty()
      || userId.empty()
      || faceToken.empty()) {
    rc = -2;
    setResponse(rc, "params error",
        result);
    LOG(ERROR) << result.toStyledString();
    return rc;
  }
  rc = service.delUserFace(groupId, userId, faceToken);
  if (rc != 0) {
    setResponse(-1, "del failed", result);
  } else {
    setResponse(0, "SUCCESS", result);
  }
  LOG(INFO) << result.toStyledString();
  return rc;
}

std::vector<HttpControl>
UserControl::getMapping() {
  std::vector<HttpControl> controlList = {
    {"/face-api/v3/face/add", UserControl::userFaceAddCb},
    {"/face-api/v3/face/delete", UserControl::userFaceDelCb},
    {"/face-api/v3/user/delete", UserControl::userDelCb},
    /* remove user's faces then add new face */
    {"/face-api/v3/face/update", UserControl::userUpdateCb},
  };
  return controlList;
}


/* add face to user*/
int UserControl::userFaceAddCb(
    const Json::Value &root,
    Json::Value &result) {
  int rc = 0;
  int len = 0;
  FaceService &service = FaceService::getFaceService(); 
  std::string groupId;
  std::string userId;
  std::string userInfo;
  std::string data;
  JsonUtil::getJsonStringValue(root, "group_id", groupId);
  JsonUtil::getJsonStringValue(root, "user_id", userId);
  JsonUtil::getJsonStringValue(root, "user_info", userInfo);
  JsonUtil::getJsonStringValue(root, "image", data);
  if (data.empty() ||
      groupId.empty() ||
      userId.empty()) {
    setResponse(-1, "param error", result);
    LOG(ERROR) << result.toStyledString();
    return -1;
  }
  int faceNum = 0;
  LOG(INFO) << "addUser :" << userId << "groupid:" << groupId << "imageLen:" << data.length(); 
  FaceUpdateResult updateResult;
  rc = service.addUserFace(groupId, userId, userInfo, data, updateResult);
  std::string faceToken = updateResult.faceToken;
  if (rc != 0 || faceToken.length() == 0) {
    setResponse(-1, "add user failed",
        result);
    LOG(ERROR) << result.toStyledString();
    return -1;
  }
  Json::Value searchResult;
  searchResult["face_token"] = faceToken;
  result["result"] = searchResult;
  result["error_code"] = "0";
  result["error_msg"] = "SUCCESS";
  LOG(INFO) <<"face add:" <<  result.toStyledString();
  return 0;
}

/* remove user's faces then add new face */
int UserControl::userUpdateCb(
    const Json::Value &root,
    Json::Value &result) {
  int rc = 0;
  int len = 0;
  FaceService &service = FaceService::getFaceService(); 
  std::string groupId;
  std::string userId;
  std::string userInfo;
  std::string data;
  JsonUtil::getJsonStringValue(root, "group_id", groupId);
  JsonUtil::getJsonStringValue(root, "user_id", userId);
  JsonUtil::getJsonStringValue(root, "user_info", userInfo);
  JsonUtil::getJsonStringValue(root, "image", data);
  if (data.empty() ||
      groupId.empty() ||
      userId.empty()) {
    setResponse(-1, "param error", result);
    LOG(ERROR) << result.toStyledString();
    return -1;
  }
  int faceNum = 0;
  LOG(INFO) << "updateUser :" << userId << "groupid:" << groupId << "imageLen:" << data.length(); 
  FaceUpdateResult updateResult;
  rc = service.updateUserFace(groupId, userId, userInfo, data, updateResult);
  if (rc != 0 || updateResult.faceToken.length() == 0) {
    setResponse(-1, "update user failed",
        result);
    LOG(ERROR) << result.toStyledString();
    return -1;
  }
  Json::Value &baiduResult = result;
  Json::Value faceResult;
  faceResult["face_token"] = updateResult.faceToken;
  Json::Value location;
  location["left"] = updateResult.location.x;
  location["top"] = updateResult.location.y;
  location["width"] = updateResult.location.width;
  location["height"] = updateResult.location.height;
  location["rotation"] = updateResult.location.rotation;
  faceResult["location"] = location;
  baiduResult["error_code"] = "0";
  baiduResult["error_msg"] = "SUCCESS";
  baiduResult["result"] = faceResult;
  LOG(INFO) <<"face update:" <<  baiduResult.toStyledString();
  return 0;
}

/*delete user*/
int UserControl::userDelCb(
    const Json::Value &root,
    Json::Value &result) {
  int rc = 0;
  int len = 0;
  FaceService &service = FaceService::getFaceService(); 
  std::string groupId;
  std::string userId;
  JsonUtil::getJsonStringValue(root, "group_id", groupId);
  JsonUtil::getJsonStringValue(root, "user_id", userId);
  if (groupId.empty() || userId.empty()) {
    setResponse(-1, "param error",
        result);
    LOG(ERROR) << result.toStyledString();
    return -1;
  }
  rc = service.delUser(groupId, userId);
  if (rc != 0) {
    setResponse(-1, "del user failed",
        result);
    LOG(ERROR) << result.toStyledString();
    return rc;
  }
  setResponse(0, "SUCCESS", result);
  LOG(INFO) << result.toStyledString();
  return rc;
}

}
