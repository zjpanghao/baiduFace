#include "featureBufferRedis.h"
#include "image_base64.h"
#include "predis/redis_pool.h"
#include "predis/redis_cmd.h"
#include "predis/redis_control.h"
namespace kface {
#define BAIDU_FEATURE_KEY "baiduFeature"
FeatureBufferRedis::FeatureBufferRedis(std::shared_ptr<RedisPool> redisPool) : redisPool_(redisPool){
}

std::shared_ptr<FaceBuffer> FeatureBufferRedis::getBuffer(const std::string &faceToken) {
  RedisControlGuard guard(redisPool_.get());
  std::shared_ptr<RedisControl> control = guard.GetControl();
  if (control == nullptr) {
    return nullptr;
  }
  std::string featureBase64;
  std::string key = BAIDU_FEATURE_KEY;
  key += faceToken;
  control->GetValue(key, &featureBase64);
  if (featureBase64.empty()) {
    return nullptr;
  }
  int len = 0;
  std::string data = ImageBase64::decode(featureBase64.c_str(), featureBase64.length(), len);
  if (len != FACE_VEC_SIZE * sizeof(float)) {
    return nullptr;
  }
  std::shared_ptr<FaceBuffer> buffer(new FaceBuffer());
  buffer->feature.assign((float*)&data[0], (float*)&data[0] + FACE_VEC_SIZE);
  return buffer;
}

void FeatureBufferRedis::addBuffer(const std::string &faceToken, std::shared_ptr<FaceBuffer> buffer) {
  RedisControlGuard guard(redisPool_.get());
  std::shared_ptr<RedisControl> control = guard.GetControl();
  if (control == nullptr) {
    return;
  }    
  std::string featureBase64 = ImageBase64::encode((unsigned char*)&buffer->feature[0], buffer->feature.size() * sizeof(float));
  std::string key = BAIDU_FEATURE_KEY;
  key += faceToken;
  control->SetExValue(key, 60, featureBase64);
}

} // namespace kface
  
