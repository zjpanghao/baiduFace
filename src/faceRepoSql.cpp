#include <fstream>
#include "faceRepoSql.h"
#include "faceAgent.h"
#include <glog/logging.h>
#include <sstream>
#include "util.h"
#include "pbase64/base64.h"

//#define DEFAULT_SAVE_NAME "faces.db"
//facelib db

namespace kface {
  FaceLibRepo::FaceLibRepo(std::shared_ptr<DBPool> pool):pool_(pool) {
  }

  int FaceLibRepo::loadPersonFaces(const std::string &name, std::list<PersonFace> &faces) {
    int rc = 0;
    int len = 0;
    const char *feature = NULL;
    std::string data;
    Connection_T conn; 
    DBPoolGuard guard(pool_, &conn);
    if (conn == NULL) {
      return -1;
    }
    ResultSet_T r;
    TRY {
      PreparedStatement_T p = Connection_prepareStatement(conn,
          "SELECT group_id, user_id, user_name, face_token, feature from face_lib");
      r = PreparedStatement_executeQuery(p);
    } CATCH(SQLException) {
      LOG(ERROR) << "get chat error:" << Exception_frame.message;
      rc = -1;
    }
    END_TRY;
    if (rc != 0) {
      return rc;
    }
    while (ResultSet_next(r)) {
      PersonFace face;
      face.groupId = ResultSet_getString(r, 1);
      face.userId = ResultSet_getString(r, 2);
      if (ResultSet_getString(r, 3) == NULL) {
        LOG(INFO) << "null";
        continue;
      }
      face.userName = ResultSet_getString(r, 3);
      face.image = std::make_shared<ImageFace>();
      face.image->faceToken = ResultSet_getString(r, 4);
      feature = ResultSet_getString(r, 5);
      std::vector<unsigned char> vec;
      Base64::getBase64().decode(std::string(feature), vec);
      if (vec.size() != FACE_VEC_SIZE  *sizeof(float)) {
        continue;
      }
      face.image->feature.assign((float*)&vec[0], (float*)(&vec[FACE_VEC_SIZE* sizeof(float)]));
      faces.push_back(face);
    }
    return rc;
  }

  int FaceLibRepo::addUserFace(const PersonFace &face) {
    int rc = 0;
    Connection_T conn;//pool_->GetConnection();
    DBPoolGuard guard(pool_, &conn);
    if (conn == NULL) {
      return -1;
    }
    Connection_execute(conn, "set names utf8");
    PreparedStatement_T p = Connection_prepareStatement(conn,
        "insert into face_lib(group_id, user_id, user_name, face_token, feature) values(?, ?, ?, ?, ?)");
    PreparedStatement_setString(p, 1, face.groupId.c_str());
    PreparedStatement_setString(p, 2, face.userId.c_str());
    PreparedStatement_setString(p, 3, face.userName.c_str());
    PreparedStatement_setString(p, 4, face.image->faceToken.c_str());
    std::string featureBase64;
    LOG(INFO) << face.image->feature.size();
    std::vector<unsigned char> vec((unsigned char*)&face.image->feature[0], (unsigned char*) &face.image->feature[FACE_VEC_SIZE]);
      Base64::getBase64().encode(vec, featureBase64);
    PreparedStatement_setString(p, 5, featureBase64.c_str());
    TRY {
      PreparedStatement_execute(p);
    } CATCH(SQLException) {
      LOG(ERROR) << "insert face error" << "user_id:" << face.userId <<" "<< Exception_frame.message;
      rc = -2;
    }
    END_TRY;
    return rc;
  }

  int FaceLibRepo::delUserFace(const PersonFace &face) {
    int rc = 0;
    Connection_T conn;// = pool_->GetConnection();
    DBPoolGuard guard(pool_, &conn);
    if (conn == NULL) {
      return -1;
    }
    Connection_execute(conn, "set names utf8");
    PreparedStatement_T p = Connection_prepareStatement(conn,
        "delete from face_lib where group_id=? and user_id=? and face_token=?");
    PreparedStatement_setString(p, 1, face.groupId.c_str());
    PreparedStatement_setString(p, 2, face.userId.c_str());
    PreparedStatement_setString(p, 3, face.image->faceToken.c_str());
    TRY {
      PreparedStatement_execute(p);
    } CATCH(SQLException) {
      LOG(ERROR) << "del face error" << "user_id:" << face.userId <<" "<< Exception_frame.message;
      rc = -2;
    }
    END_TRY;
    return rc;
  }

  int FaceLibRepo::delUser(const PersonFace &face) {
    int rc = 0;
    Connection_T conn;// = pool_->GetConnection();
    DBPoolGuard guard(pool_, &conn);
    if (conn == NULL) {
      return -1;
    }
    Connection_execute(conn, "set names utf8");
    PreparedStatement_T p = Connection_prepareStatement(conn,
        "delete from face_lib where group_id=? and user_id=?");
    PreparedStatement_setString(p, 1, face.groupId.c_str());
    PreparedStatement_setString(p, 2, face.userId.c_str());
    TRY {
      PreparedStatement_execute(p);
    } CATCH(SQLException) {
      LOG(ERROR) << "del user error" << "user_id:" << face.userId <<" "<< Exception_frame.message;
      rc = -2;
    }
    END_TRY;
    return rc;
  }

}// namespace FaceLibRepo
