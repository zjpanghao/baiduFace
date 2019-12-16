#ifndef INCLUDE_USER_CONTROL_H
#define INCLUDE_USER_CONTROL_H
#include <vector>
#include "httpUtil.h"
#include "evdrv/urlMap.h"
struct evhttp_request;
namespace kface {
class UserControl : public UrlMap {
  public:
    static int userFaceAddCb(
        const Json::Value &root,\
        Json::Value &result);

    static int userFaceDelCb(
        const Json::Value &root,\
        Json::Value &result);

    static int userDelCb(
        const Json::Value &root,
        Json::Value &result);

    static int userUpdateCb(
    const Json::Value &root,
    Json::Value &result);

    std::vector<HttpControl>
      getMapping() override;
};

}
#endif
