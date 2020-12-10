#ifndef INCLUDE_USER_CONTROL_H
#define INCLUDE_USER_CONTROL_H
#include <vector>
#include "httpUtil.h"
#include "evdrv/generalControl.h"
struct evhttp_request;
namespace kface {
class UserControl : public GeneralControl {
  public:
    static int userFaceAddCb(
        const pson::Json::Value &root,\
        pson::Json::Value &result);

    static int userFaceDelCb(
        const pson::Json::Value &root,\
        pson::Json::Value &result);

    static int userDelCb(
        const pson::Json::Value &root,
        pson::Json::Value &result);

    static int userUpdateCb(
    const pson::Json::Value &root,
    pson::Json::Value &result);

    int userListCb(
        const pson::Json::Value &root,
        pson::Json::Value &result);

    std::vector<HttpControl>
      getMapping() override;
};

}
#endif
