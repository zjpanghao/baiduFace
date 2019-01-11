#ifndef INCLUDE_PERSON_FACE_H
#define INCLUDE_PERSON_FACE_H
#define MAX_IMAGE_SIZE 10 * 1024 * 1024
#define FACE_FEATURE_SIZE 512
#include <string>
#include <json/json.h>
#include <list>
#include <vector>
#include <memory>
#include "faceEntity.h"

namespace kface {
struct PersonFace {
  std::string appName;
  std::string groupId;
  std::string userId;
  std::string userName;
  std::shared_ptr<ImageFace> image;
};

void savePersonFaces(const std::list<PersonFace> &faces); 
void savePersonFaces(const std::list<PersonFace> &faces, const std::string &fname); 
void loadPersonFaces(std::list<PersonFace> &faces); 
void loadPersonFaces(const std::string &name, std::list<PersonFace> &faces); 
void flushFaces();

}

#endif

