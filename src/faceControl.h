#ifndef INCLUDE_FACE_CONTROL_H
#define INCLUDE_FACE_CONTROL_H
#include <vector>
#include "evdrv/urlMap.h"

namespace kface {
class FaceControl : public UrlMap {
  public:
    FaceControl() = default;

    static int faceDebugCb(
        const Json::Value &root, 
        Json::Value &result);

    static int faceMatchCb(
        const Json::Value &root, 
        Json::Value &result); 

    static int faceIdentifyCb(
        const Json::Value &root,
        Json::Value &result);
    static int faceDetectCb(
        const Json::Value &root, 
        Json::Value &result);
    std::vector<HttpControl>
      getMapping() override;
};

} // namespace kface
#endif
