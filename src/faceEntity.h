#ifndef INCLUDE_FACE_ENTITY_H
#define INCLUDE_FACE_ENTITY_H
namespace kface {
struct ImageFace {
  std::string data;
  std::string faceToken;
  std::vector<float> feature;
};
}
#endif
