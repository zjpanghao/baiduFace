#ifndef INCLUDE_HTTP_UTIL_H
#define INCLUDE_HTTP_UTIL_H
#include <map>
#include <string>

#define HTTP_RECV_BUF_SIZE 500*1024
struct evhttp_request;
struct evbuffer;
std::string getBodyStr(struct evhttp_request *req);
void sendResponse(int errorCode, 
    std::string msg,  
    struct evhttp_request *&req, 
    evbuffer *&response);

template<class vvalue>
void sendResponseResult(int errorCode, 
    std::string msg,
    const std::map<std::string, vvalue> &paraMap,
    struct evhttp_request *&req, 
    evbuffer *&response); 

struct HttpControl {
  std::string url;
  void (*cb)(evhttp_request *reg, void *arg);
};

#endif
