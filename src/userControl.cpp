#include "userControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

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
namespace kface {

void userFaceDelCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  int len = 0;
  Json::Value root;
  Json::Value delResult;
  Json::Value items;
  Json::Reader reader;
  int decodeLen = 0;
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
  if (root["group_id"].isNull()
      || root["user_id"].isNull()
      || root["face_token"].isNull()) {
    rc = -2;
    sendResponse(rc, "method not support", req, response);
    return;
  }
  std::string groupId = root["group_id"].asString();
  std::string userId = root["user_id"].asString();
  std::string faceToken = root["fack_token"].asString();
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

void userFaceAddCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  int len = 0;
  Json::Value root;
  Json::Value faceResult;
  Json::Value items;
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
  std::string data = root["image"].isNull() ? "" : root["image"].asString();
  int faceNum = 0;
  std::string userId =  (root["user_id"].isNull() ? "1" : root["user_id"].asString());
  std::string groupId =  (root["group_id"].isNull() ? "1" : root["group_id"].asString());
  std::string userInfo =  (root["user_info"].isNull() ? "1" : root["user_info"].asString());
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

void userDelCb(struct evhttp_request *req, void *arg) {
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
  std::string userId =  (root["user_id"].isNull() ? "1" : root["user_id"].asString());
  std::string groupId =  (root["group_id"].isNull() ? "1" : root["group_id"].asString());
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

void initUserControl(std::vector<HttpControl> &controls) {
  std::vector<HttpControl> controlList = {
    {"/face-api/v3/face/add", userFaceAddCb},
    {"/face-api/v3/face/delete", userFaceDelCb},
    {"/face-api/v3/user/delete", userDelCb},
  };
  for (HttpControl &control : controlList) {
    controls.push_back(control);
  }
}

}
