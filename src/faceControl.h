#ifndef INCLUDE_FACE_CONTROL_H
#define INCLUDE_FACE_CONTROL_H
#include <vector>
#include "evdrv/generalControl.h"
namespace kunyan {
  class Config;
}

namespace kface {
class FaceControl : public GeneralControl{
  public:
    FaceControl() = default;

    static int faceDebugCb(
        const pson::Json::Value &root, 
        pson::Json::Value &result);

    static int faceMatchCb(
        const pson::Json::Value &root, 
        pson::Json::Value &result); 

    static int faceIdentifyCb(
        const pson::Json::Value &root,
        pson::Json::Value &result);
    static int faceDetectCb(
        const pson::Json::Value &root, 
        pson::Json::Value &result);
    std::vector<HttpControl>
      getMapping() override;
    int init(const kunyan::Config &config) override;
};

} // namespace kface
#endif
