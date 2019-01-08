#include <fstream>
#include "faceRepo.h"
#include "faceAgent.h"
#include <glog/logging.h>
#include <sstream>
#define DEFAULT_SAVE_NAME "faces.db"
namespace kface {

  void savePersonFaces(const std::list<PersonFace> &faces) {
    savePersonFaces(faces, DEFAULT_SAVE_NAME);
  }

  void loadPersonFaces(std::list<PersonFace> &faces) {
    loadPersonFaces(DEFAULT_SAVE_NAME, faces);
  }
  void loadPersonFaces(const std::string &name, std::list<PersonFace> &faces) {
    std::ifstream imageFile(name, std::ifstream::in);
    std::stringstream buffer;
    buffer << imageFile.rdbuf();
    imageFile.close();
    std::string content(buffer.str());
    LOG(INFO) << content.size();
    Json::Reader reader;
    Json::Value root;
    bool f = reader.parse(content, root, true);
    LOG(INFO) << "parse:" << f;
    if (!f) {
      LOG(ERROR) << reader.getFormatedErrorMessages();
    }
    LOG(INFO) << "db size :" << root.size();
    for (int i = 0; i < root.size(); i++) {
      PersonFace face;
      face.faceToken = root[i]["faceToken"].asString();
      face.userId = root[i]["userId"].asString();
      face.groupId = root[i]["groupId"].asString();
      face.appName = root[i]["appName"].asString();
      Json::Value val = root[i]["feature"];
      for (int j = 0; j < val.size(); j++) {
        face.feature.push_back(val[j].asDouble());
      }
      faces.push_back(face);
    }
  }

  void savePersonFaces(const std::list<PersonFace> &faces, const std::string &fname) {
    Json::Value root;
    int index = 0;
    for (const PersonFace &face : faces) {
      if (face.faceToken == "") {
        continue;
      }
      Json::Value image;
      image["appName"] = face.appName;    
      image["groupId"] = face.groupId;
      image["userId"] = face.userId;
      image["userName"] = face.userName;
      image["faceToken"] = face.faceToken;
      Json::Value val;
      int i = 0;
      for (float v : face.feature) {
        val[i++] = Json::Value(v);
      }
      image["feature"] = val;
      root[index++] = image;
    }
    if (root.isNull()) {
      return;
    }
    std::string res = root.toStyledString();
    LOG(INFO) << res;
    if (res.size() < 10) {
      return;
    }
    std::ofstream imageFile(fname, std::ofstream::out);
    imageFile << res;
    imageFile.close();
  }

  void flushFaces() {
    FaceAgent &agent = FaceAgent::getFaceAgent();
    std::list<PersonFace> faces;
    agent.getDefaultPersonFaces(faces);
    savePersonFaces(faces);
    LOG(INFO) << "flush :" << faces.size() << " to db";
  }
}
