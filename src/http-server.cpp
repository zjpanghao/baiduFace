/*
  A trivial static http webserver using Libevent's evhttp.

  This is not the best code in the world, and it does some fairly stupid stuff
  that you would never want to do in a production webserver. Caveat hackor!

 */

/* Compatibility for possible missing IPv6 declarations */
//#include "../util-internal.h"

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

#ifdef _WIN32
#ifndef stat
#define stat _stat
#endif
#ifndef fstat
#define fstat _fstat
#endif
#ifndef open
#define open _open
#endif
#ifndef close
#define close _close
#endif
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#endif
#include <thread>
#include <vector>
#include <glog/logging.h>
#include <iterator>
#include <regex>
#include "faceAgent.h"
#include "faceService.h"
#include "httpUtil.h"
#include "userControl.h"

#define MAX_IMG_SIZE 1024*1024*50
using namespace kface;

char uri_root[512];

static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postscript" },
	{ NULL, NULL },
};

using namespace cv;

/* Try to guess a good content-type for 'path' */
static const char *
guess_content_type(const char *path)
{
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!evutil_ascii_strcasecmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}

void logFile(char *name,  char*data, int rows, int cols) {
  Mat m(rows,cols, CV_8UC3);
  std::cout << m.channels();
  MatIterator_<Vec3b> it = m.begin<Vec3b>();
  while (it != m.end<Vec3b>()){
    
      (*it)[2] = *data++;
      (*it)[1] = *data++;
     (*it)[0] = *data++;
  
    it++;
  }
  imwrite(name, m);
}

static void faceIdentifyCb(struct evhttp_request *req, void *arg) {
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

static void faceDetectCb(struct evhttp_request *req, void *arg) {
 
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
    //sendResponse(rc, "detect face error", req, response);
    //return;
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
      //item["angle"] =  it->trackInfo.box.mAngle;
      //item["conf"] =  it->trackInfo.box.mConf;
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
  #if 0
      Json::Value land;
      int inx = 0;
      for (int v : it->trackInfo.landmarks) {
        land[inx++] = v;
      }
      item["landmarks"] = land;
#endif
      //item["face_id"] = (unsigned int)it->trackInfo.face_id; 
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

static void
syntax(void)
{
	fprintf(stdout, "Syntax: http-server <docroot>\n");
}

void httpThread(void *param) {
  LOG(INFO) << "new Thread";
  struct event_base *base = (struct event_base *)param;
  event_base_dispatch(base);
}
void ev_server_start_multhread(int port, int nThread) {
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return;
  evutil_socket_t fd;
  struct evconnlistener *listener = NULL;
  for (int i = 0; i < nThread; i++) {
    struct event_base *base = event_base_new();
  	if (!base) {
  		LOG(ERROR) << "Couldn't create an event_base: exiting";
  		return;
  	}

      /* Create a new evhttp object to handle requests. */
  	struct evhttp *http = evhttp_new(base);
  	if (!http) {
  		LOG(ERROR) << "couldn't create evhttp. Exiting.";
  		return;
  	}
     /* The /dump URI will dump all requests to stdout and say 200 ok. */
    //evhttp_set_cb(http, "/identify", identifyCb, (void*)arg);
    if (i == 0) {
      struct evhttp_bound_socket *bound;
      bound = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
      if (!bound) {
        return;
      }
      fd = evhttp_bound_socket_get_fd(bound);
    } else {
      evhttp_accept_socket(http, fd);
    }

   
    std::thread t(httpThread, base);
    t.detach();    
  }
}


int ev_server_start(int port)
{
	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

#ifdef _WIN32
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (1);
#endif

	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

	/* Create a new evhttp object to handle requests. */
	http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

	/* The /dump URI will dump all requests to stdout and say 200 ok. */
  evhttp_set_cb(http, "/face-api/v3/face/detect", faceDetectCb, NULL);
  evhttp_set_cb(http, "/face-api/v3/face/identify", faceIdentifyCb, NULL);
  std::vector<HttpControl> controls;
  initUserControl(controls);
  for (HttpControl control : controls) {
    evhttp_set_cb(http, control.url.c_str(), control.cb, NULL);
  }

	/* We want to accept arbitrary requests, so we need to set a "generic"
	 * cb.  We can also add callbacks for specific paths. */
	//evhttp_set_gencb(http, send_document_cb, argv[1]);

	/* Now we tell the evhttp what port to listen on */
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
		    (int)port);
		return 1;
	}

	{
		/* Extract and display the address we're listening on. */
		struct sockaddr_storage ss;
		evutil_socket_t fd;
		ev_socklen_t socklen = sizeof(ss);
		char addrbuf[128];
		void *inaddr;
		const char *addr;
		int got_port = -1;
		fd = evhttp_bound_socket_get_fd(handle);
		memset(&ss, 0, sizeof(ss));
		if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
			perror("getsockname() failed");
			return 1;
		}
		if (ss.ss_family == AF_INET) {
			got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
			inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
		} else if (ss.ss_family == AF_INET6) {
			got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
			inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
		} else {
			fprintf(stderr, "Weird address family %d\n",
			    ss.ss_family);
			return 1;
		}
		addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
		    sizeof(addrbuf));
		if (addr) {
			printf("Listening on %s:%d\n", addr, got_port);
			evutil_snprintf(uri_root, sizeof(uri_root),
			    "http://%s:%d",addr,got_port);
		} else {
			fprintf(stderr, "evutil_inet_ntop failed\n");
			return 1;
		}
	}

	event_base_dispatch(base);

	return 0;
}
