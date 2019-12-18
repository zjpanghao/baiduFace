#ifndef INCLUDE_FEATUREBUFFER_MEMORY_H
#define INCLUDE_FEATUREBUFFER_MEMORY_H
#include <memory>
#include <unordered_map>
#include "faceEntity.h"
#include "featureBuffer.h"
#include <mutex>
#include <thread>

namespace kface {
class  FeatureBufferMemory : public FeatureBuffer {
 typedef std::unordered_map<std::string, std::shared_ptr<FaceBuffer>> FaceFeatureMap;
 public:
  FeatureBufferMemory() = default;
  virtual bool init() override;
  virtual std::shared_ptr<FaceBuffer> getBuffer(const std::string &faceToken) override;
  virtual void addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer) override;

 private:
  void clear();
  FaceFeatureMap faceFeatureMap_;
  std::mutex lock_;
  std::shared_ptr<std::thread>  thd_;
  int mdayOld_ = 0;
};

} // namespace kface
#endif
