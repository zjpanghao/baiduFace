/*
  A trivial static http webserver using Libevent's evhttp.

  This is not the best code in the world, and it does some fairly stupid stuff
  that you would never want to do in a production webserver. Caveat hackor!

 */

/* Compatibility for possible missing IPv6 declarations */
//#include "../util-internal.h"
#include "httpUtil.h"

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

void sendResponse(int errorCode, 
    std::string msg,  
    struct evhttp_request *&req, 
    evbuffer *&response) {
  Json::Value root;
  std::stringstream ss;
  ss << errorCode;
  root["error_code"] = ss.str();
  root["error_msg"] = msg;
  std::string s = root.toStyledString();
  evbuffer_add_printf(response, "%s", s.c_str());
  LOG(ERROR) << s;
  evhttp_send_reply(req, 200, "OK", response);
}

template<class vvalue>
void sendResponseResult(int errorCode, 
    std::string msg,
    const std::map<std::string, vvalue> &paraMap,
    struct evhttp_request *&req, 
    evbuffer *&response) {
  Json::Value root;
  std::stringstream ss;
  ss << errorCode;
  root["error_code"] = ss.str();
  root["error_msg"] = msg;
  for (auto it = paraMap.begin(); it != paraMap.end(); it++) {
    root[it->first] = it->second;
  }
  std::string s = root.toStyledString();
  evbuffer_add_printf(response, "%s", s.c_str());
  evhttp_send_reply(req, 200, "OK", response);
}

std::string getBodyStr(struct evhttp_request *req) {
  struct evbuffer *buf = evhttp_request_get_input_buffer(req);
  std::string result = "";
  //if (evbuffer_get_length(buf) >= MAX_RECV_SIZE  - 1024) {
	//  return "";
  //}
  
  result.reserve(HTTP_RECV_BUF_SIZE);
  int rc = 0;
  int len = 0;
  while (evbuffer_get_length(buf)) {
    int n;
    char cbuf[128];
    n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
    if (n > 0) {
      len += n;
      //if (len >= MAX_RECV_SIZE  - 1024) {
      //  return "";
      //}
      result += std::string(cbuf, n);
    }
  }
  return result;
}
