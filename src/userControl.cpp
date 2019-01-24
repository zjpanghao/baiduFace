#include "userControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

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
#include "httpUtil.h"
#include "util.h"

namespace kface {
/* delete user face */
static void userFaceDelCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  int len = 0;
  Json::Value root;
  Json::Value delResult;
  Json::Reader reader;
  FaceService &service = FaceService::getFaceService(); 
  evbuffer *response = evbuffer_new();
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    rc = -1;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  std::string body = getBodyStr(req);
  if (!reader.parse(body, root)) {
    rc = -3;
    sendResponse(rc, "parse error", req, response);
    return;
  }
  std::string groupId;
  std::string userId;
  std::string faceToken;
  getJsonString(root, "group_id", groupId);
  getJsonString(root, "user_id", userId);
  getJsonString(root, "face_token", faceToken);
  if (groupId.empty()
      || userId.empty()
      || faceToken.empty()) {
    rc = -2;
    sendResponse(rc, "params error", req, response);
    return;
  }
  rc = service.delUserFace(groupId, userId, faceToken);
  if (rc != 0) {
    delResult["error_code"] = "-1";
    delResult["error_msg"]  = "del failed";  
  } else {
    delResult["error_code"] = "0";
    delResult["error_msg"]  = "SUCCESS";  
  }
  LOG(INFO) << delResult.toStyledString();
  evbuffer_add_printf(response, "%s", delResult.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

/* add face to user*/
static void userFaceAddCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  int len = 0;
  Json::Value root;
  Json::Value faceResult;
  Json::Reader reader;
  FaceService &service = FaceService::getFaceService(); 
  evbuffer *response = evbuffer_new();
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    rc = -1;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  std::string body = getBodyStr(req);
  if (!reader.parse(body, root)) {
    rc = -3;
    sendResponse(rc, "parse error", req, response);
    return;
  }
  std::string groupId;
  std::string userId;
  std::string userInfo;
  std::string data;
  getJsonString(root, "group_id", groupId);
  getJsonString(root, "user_id", userId);
  getJsonString(root, "user_info", userInfo);
  getJsonString(root, "image", data);
  if (data.empty() ||
      groupId.empty() ||
      userId.empty()) {
    sendResponse(-1, "param error", req, response);
    return;
  }
  int faceNum = 0;
  LOG(INFO) << "addUser :" << userId << "groupid:" << groupId << "imageLen:" << data.length(); 
  std::string faceToken;
  rc = service.addUserFace(groupId, userId, userInfo, data, faceToken);
  if (rc != 0 || faceToken.length() == 0) {
    sendResponse(-1, "add user failed", req, response);
    return;
  }
  Json::Value results;
  Json::Value result;
  result["face_token"] = faceToken;
  results["result"] = result;
  results["error_code"] = "0";
  results["error_msg"] = "SUCCESS";
  LOG(INFO) <<"face add:" <<  results.toStyledString();
  evbuffer_add_printf(response, "%s", results.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

/* remove user's faces then add new face */
static void userUpdateCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  int len = 0;
  Json::Value root;
  Json::Value faceResult;
  Json::Reader reader;
  FaceService &service = FaceService::getFaceService(); 
  evbuffer *response = evbuffer_new();
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    rc = -1;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  std::string body = getBodyStr(req);
  if (!reader.parse(body, root)) {
    rc = -3;
    sendResponse(rc, "parse error", req, response);
    return;
  }
  std::string groupId;
  std::string userId;
  std::string userInfo;
  std::string data;
  getJsonString(root, "group_id", groupId);
  getJsonString(root, "user_id", userId);
  getJsonString(root, "user_info", userInfo);
  getJsonString(root, "image", data);
  if (data.empty() ||
      groupId.empty() ||
      userId.empty()) {
    sendResponse(-1, "param error", req, response);
    return;
  }
  int faceNum = 0;
  LOG(INFO) << "updateUser :" << userId << "groupid:" << groupId << "imageLen:" << data.length(); 
  FaceUpdateResult updateResult;
  rc = service.updateUserFace(groupId, userId, userInfo, data, updateResult);
  if (rc != 0 || updateResult.faceToken.length() == 0) {
    sendResponse(-1, "update user failed", req, response);
    return;
  }
  Json::Value result;
  result["face_token"] = updateResult.faceToken;
  Json::Value location;
  location["left"] = updateResult.location.x;
  location["top"] = updateResult.location.y;
  location["width"] = updateResult.location.width;
  location["height"] = updateResult.location.height;
  location["rotation"] = updateResult.location.rotation;
  result["location"] = location;
  result["error_code"] = "0";
  result["error_msg"] = "SUCCESS";
  LOG(INFO) <<"face update:" <<  result.toStyledString();
  evbuffer_add_printf(response, "%s", result.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

/*delete user*/
static void userDelCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  int len = 0;
  Json::Value root;
  Json::Reader reader;
  FaceService &service = FaceService::getFaceService(); 
  evbuffer *response = evbuffer_new();
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    rc = -1;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  std::string body = getBodyStr(req);
  if (!reader.parse(body, root)) {
    rc = -3;
    sendResponse(rc, "parse error", req, response);
    return;
  }
  std::string groupId;
  std::string userId;
  getJsonString(root, "group_id", groupId);
  getJsonString(root, "user_id", userId);
  if (groupId.empty() || userId.empty()) {
    sendResponse(-1, "param error", req, response);
    return;
  }
  rc = service.delUser(groupId, userId);
  if (rc != 0) {
    sendResponse(-1, "del user failed", req, response);
    return;
  }
  Json::Value result;
  result["error_code"] = "0";
  result["error_msg"] = "SUCCESS";
  LOG(INFO) << result.toStyledString();
  evbuffer_add_printf(response, "%s", result.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

/*register user interface*/
void initUserControl(std::vector<HttpControl> &controls) {
  std::vector<HttpControl> controlList = {
    {"/face-api/v3/face/add", userFaceAddCb},
    {"/face-api/v3/face/delete", userFaceDelCb},
    {"/face-api/v3/user/delete", userDelCb},
    /* remove user's faces then add new face */
    {"/face-api/v3/face/update", userUpdateCb},
  };
  for (HttpControl &control : controlList) {
    controls.push_back(control);
  }
}

}
