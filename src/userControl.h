#ifndef INCLUDE_USER_CONTROL_H
#define INCLUDE_USER_CONTROL_H
namespace kface {
struct evhttp_request;
void faceAddCb(struct evhttp_request *req, void *arg);
}
#endif
