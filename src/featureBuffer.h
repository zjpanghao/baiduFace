#ifndef INCLUDE_FEATUREBUFFER_H
#define INCLUDE_FEATUREBUFFER_H
#include <memory>
#include "faceEntity.h"
namespace kface {
class  FeatureBuffer {
 public:
   virtual ~FeatureBuffer() = default;
   virtual bool init(){ return true;}
   virtual std::shared_ptr<FaceBuffer> getBuffer(const std::string &faceToken) = 0;
  virtual void addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer) = 0;
};

} // namespace kface
#endif
