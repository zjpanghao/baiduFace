#ifndef INCLUDE_FEATUREBUFFER_REDIS_H
#define INCLUDE_FEATUREBUFFER_REDIS_H
#include <memory>
#include "predis/redis_pool.h"
#include "faceEntity.h"
#include "featureBuffer.h"
namespace kface {
class FeatureBufferRedis : public FeatureBuffer {
 public:
  FeatureBufferRedis(std::shared_ptr<RedisPool> redisPool);
  
  std::shared_ptr<FaceBuffer> getBuffer(const std::string &faceToken) override;
  void addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer) override;
  
 private:
  //std::map<std::string, std::shared_ptr<FaceBuffer> > faceBuffers[2];
  std::shared_ptr<RedisPool> redisPool_{nullptr}; 
};

} // namespace kface
#endif
