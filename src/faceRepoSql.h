#ifndef INCLUDE_FACELIBREPO_H
#define INCLUDE_FACELIBREPO_H
#define MAX_IMAGE_SIZE 10 * 1024 * 1024
#define FACE_FEATURE_SIZE 512
#include <string>
#include <json/json.h>
#include <list>
#include <vector>
#include <memory>
#include "faceEntity.h"
#include "db/dbpool.h"

namespace kface {

struct PersonFace;
class FaceLibRepo{
  public:
    FaceLibRepo(std::shared_ptr<DBPool> pool);
    int queryGroupUids(
      const std::string &gourpId,
      std::vector<std::string> &uids);
    int loadPersonFaces(const std::string &name, std::list<PersonFace> &faces);
    int  delUserFace(const PersonFace &face);
    int  addUserFace(const PersonFace &face);
    int  delUser(const PersonFace &face);
    std::shared_ptr<DBPool> getPool() {
      return pool_;
    }

  private:
    std::shared_ptr<DBPool> pool_;
};

} // namespace kface
#endif

