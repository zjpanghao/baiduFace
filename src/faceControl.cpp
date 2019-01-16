#include "faceControl.h"
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

namespace kface {
	void faceDetectCb(struct evhttp_request *req, void *arg) {
	  const char *cmdtype;
	  struct evkeyvalq *headers;
	  struct evkeyval *header;
	  struct evbuffer *buf;
	  int rc = 0;
	  int len = 0;
	  std::vector<FaceDetectResult> result;
	  auto it = result.begin();
	  Json::Value root;
	  Json::Value faceResult;
	  Json::Value items;
	  Json::Reader reader;
	  int decodeLen = 0;
	  FaceService &service = FaceService::getFaceService(); 
	  std::string decodeData; 
	  std::vector<unsigned char> cdata;
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
	  std::stringstream num;
	  int faceNum = 0;
	  num << (root["max_face_num"].isNull() ? "1" : root["max_face_num"].asString());
	  num >> faceNum;
	  decodeData = ImageBase64::decode(data.c_str(), data.size(), decodeLen);
	  cdata.assign(&decodeData[0], &decodeData[0] + decodeLen);
	  
	  rc = service.detect(cdata, faceNum, result);
	  if (rc != 0) {
		rc = -4;
		result.clear();
	  }
	  it = result.begin();
	  faceResult["error_code"] = "0";
	  faceResult["error_msg"] = "SUCCESS";
	  {
		Json::Value content;
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
	  }
	  if (result.size() > 0) {
		LOG(INFO) << faceResult.toStyledString();
	  }
	  evbuffer_add_printf(response, "%s", faceResult.toStyledString().c_str());
	  evhttp_send_reply(req, 200, "OK", response);
	}


void faceIdentifyCb(struct evhttp_request *req, void *arg) {
  int rc = 0;
  int len = 0;
  Json::Value root;
  Json::Value faceResult;
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
  std::string faceToken = root["image"].asString();
  std::stringstream num;
  num  << (root["max_user_num"].isNull() ? "1" : root["max_user_num"].asString());
  int faceNum;
  num >> faceNum;
  std::string groupIds = root["group_id_list"].asString();
  std::regex re(",");
  std::set<std::string> groupList(std::sregex_token_iterator(groupIds.begin(), groupIds.end(), re, -1),
            std::sregex_token_iterator());
  LOG(INFO) << "faceToken:" << faceToken << "gid:" << groupIds;
  std::vector<FaceSearchResult> resultList;
  rc = service.search(groupList, faceToken, faceNum, resultList);
  if (rc != 0) {
    rc = -4;
    sendResponse(rc, "no such facetoken", req, response);
    return;
  }
  faceResult["error_code"] = "0";
  for (FaceSearchResult &result : resultList) {
    Json::Value content;
    Json::Value item;
    item["user_id"] = result.userId;
    item["score"] = result.score;
    item["user_info"] = "";
    item["group_id"] = result.groupId;
    items.append(item);
    content["user_list"] = items;
    faceResult["result"] = content;
  }

  if (resultList.size() > 0) {
    faceResult["error_msg"] = "SUCCESS";
  } else {
    faceResult["error_msg"] = "match user is not found";
  }
  LOG(INFO) << faceResult.toStyledString();
  evbuffer_add_printf(response, "%s", faceResult.toStyledString().c_str());
  evhttp_send_reply(req, 200, "OK", response);
done:
  ;
}

void initFaceControl(std::vector<HttpControl> &controls) {
  std::vector<HttpControl> controlList = {
    {"/face-api/v3/face/detect", faceDetectCb},
    {"/face-api/v3/face/identify", faceIdentifyCb}
  };
  for (HttpControl &control : controlList) {
    controls.push_back(control);
  }
}

}

