#ifndef INCLUDE_USER_CONTROL_H
#define INCLUDE_USER_CONTROL_H
#include <vector>
#include "httpUtil.h"
struct evhttp_request;
namespace kface {
void initUserControl(std::vector<HttpControl> &controls); 
}
#endif
