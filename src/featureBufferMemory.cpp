#include "featureBufferMemory.h"
#include "image_base64.h"
#include <chrono>
#include <sys/time.h>
#include "timer/timer.h"
#include <glog/logging.h>

#define BAIDU_FEATURE_KEY "baiduFeature"
namespace kface {

void FeatureBufferMemory::clear() {
  time_t current = time(NULL);
  struct tm val;
  localtime_r(&current, &val);
  int mday = val.tm_mday;
  if (mday != mdayOld_) {
    std::lock_guard<std::mutex> guard(lock_);
    LOG(INFO) << "clear:" << faceFeatureMap_.size();
    faceFeatureMap_.clear();
    mdayOld_ = val.tm_mday;
  }
}

bool FeatureBufferMemory::init() {
  kun::Timer::getTimer().addFunc(120, -1, std::bind(&FeatureBufferMemory::clear, this));
  return true;
}

std::shared_ptr<FaceBuffer> FeatureBufferMemory::getBuffer(const std::string &faceToken) {
  std::lock_guard<std::mutex> guard(lock_);
  auto it = faceFeatureMap_.find(faceToken);
  if (it == faceFeatureMap_.end()) {
    return nullptr;
  }
  return it->second;
}

void FeatureBufferMemory::addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer) {
  std::lock_guard<std::mutex> guard(lock_);
  faceFeatureMap_[faceToken] = buffer;
}

} // namespace kface
  
