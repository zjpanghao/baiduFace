#ifndef INCLUDE_USER_CONTROL_H
#define INCLUDE_USER_CONTROL_H
#include <vector>
#include "httpUtil.h"
struct evhttp_request;
namespace kface {
void userFaceAddCb(struct evhttp_request *req, void *arg);
void userFaceDelCb(struct evhttp_request *req, void *arg);
void userDelCb(struct evhttp_request *req, void *arg);
void userUpdateCb(struct evhttp_request *req, void *arg);
void groupDelCb(struct evhttp_request *req, void *arg);
void initUserControl(std::vector<HttpControl> &controls); 
}
#endif
