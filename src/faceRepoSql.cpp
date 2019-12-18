#include <fstream>
#include "faceRepoSql.h"
#include "faceAgent.h"
#include <glog/logging.h>
#include <sstream>
#include "util.h"
#include "pbase64/base64.h"
#include "db/sqlTemplate.h"
#include "faceLib.h"
#include "db/preparedStmt.h"

//#define DEFAULT_SAVE_NAME "faces.db"
//facelib db

namespace kface {
  FaceLibRepo::FaceLibRepo(std::shared_ptr<DBPool> pool):pool_(pool) {
  }

  int FaceLibRepo::loadPersonFaces(const std::string &name, std::list<PersonFace> &faces) {
    std::string sql = "SELECT group_id, user_id, user_name, face_token, feature from face_lib";
    SqlTemplate sqlTemplate(pool_);
    FaceLibMapper<PersonFace> fbMapper;
    return sqlTemplate.query<PersonFace>(sql, 
        fbMapper, 
        faces);
  }

  int FaceLibRepo::addUserFace(const PersonFace &face) {
    int rc = 0;
    std::string sql = "insert into face_lib(group_id, user_id, user_name, face_token, feature) values(?, ?, ?, ?, ?)";
    PreparedStmt ps(sql);
    ps.setString(1, face.groupId);
    ps.setString(2,face.userId);
    ps.setString(3, face.userName != "" ? face.userName : "none");
    ps.setString(4, face.image->faceToken);
    std::string feature;
    std::vector<unsigned char> vec((unsigned char*)&face.image->feature[0], (unsigned char*) &face.image->feature[FACE_VEC_SIZE]);
    Base64::getBase64().encode(vec, feature);
    ps.setString(5, feature);
    SqlTemplate sqlTemplate(pool_);
    return sqlTemplate.update(ps);
  }

  int FaceLibRepo::delUserFace(const PersonFace &face) {
    std::string sql = "delete from face_lib where group_id=? and user_id=? and face_token=?";
    PreparedStmt ps(sql);
    ps.setString(1, face.groupId.c_str());
    ps.setString(2,face.userId.c_str());
    ps.setString(3, face.image->faceToken.c_str());
    SqlTemplate sqlTemplate(pool_);
    return sqlTemplate.update(ps);
  }

  int FaceLibRepo::delUser(const PersonFace &face) {
    std::string sql = "delete from face_lib where group_id=? and user_id=?";
    PreparedStmt ps(sql);
    ps.setString(1, face.groupId.c_str());
    ps.setString(2,face.userId.c_str());
    SqlTemplate sqlTemplate(pool_);
    return sqlTemplate.update(ps);
  }

}// namespace FaceLibRepo
