#ifndef INCLUDE_FACE_CONTROL_H
#define INCLUDE_FACE_CONTROL_H
#include <vector>
#include "httpUtil.h"
struct evhttp_request;
namespace kface {
void faceIdentifyCb(struct evhttp_request *req, void *arg);
void faceDetectCb(struct evhttp_request *req, void *arg);
void initFaceControl(std::vector<HttpControl> &controls); 
}

#endif
