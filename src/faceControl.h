#ifndef INCLUDE_FACE_CONTROL_H
#define INCLUDE_FACE_CONTROL_H
#include <vector>
#include "httpUtil.h"
struct evhttp_request;
namespace kface {
/* can accept image_type FACE_TOKEN & BASE64*/
void faceIdentifyCb(struct evhttp_request *req, void *arg);
/*support image type  base64*/
void faceDetectCb(struct evhttp_request *req, void *arg);
void initFaceControl(std::vector<HttpControl> &controls); 
}

#endif
