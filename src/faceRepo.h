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
#include <mongoc/mongoc.h>

namespace kface {
struct PersonFace {
  std::string appName;
  std::string groupId;
  std::string userId;
  std::string userName;
  std::shared_ptr<ImageFace> image;
};

int initRepoFaces(mongoc_client_pool_t *pool, const std::string &name);
//void savePersonFaces(const std::list<PersonFace> &faces); 
//void savePersonFaces(const std::list<PersonFace> &faces, const std::string &fname); 
void repoLoadPersonFaces(std::list<PersonFace> &faces); 
//void loadPersonFaces(const std::string &name, std::list<PersonFace> &faces); 
void flushFaces();
int repoDelUserFace(const PersonFace &face);
int repoAddUserFace(const PersonFace &face);
int repoDelUser(const PersonFace &face);
void localLoadPersonFaces(const std::string &name, std::list<PersonFace> &faces);
}

#endif

