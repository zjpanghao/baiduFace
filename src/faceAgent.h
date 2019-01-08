#ifndef INCLUDE_FACE_AGENT_H
#define INCLUDE_FACE_AGENT_H
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <list>

#define DEFAULT_APP_NAME "door"

namespace kface {
struct PersonFace;
struct ImageFace {
  std::string data;
  std::string faceToken;
  std::vector<float> feature;
};

class UserFace {
 public:
  UserFace(const std::string &userId, 
           const std::string &userName);
  int addImageFace(std::shared_ptr <ImageFace> imageFace);
  int delImageFace(const std::string faceToken);
  std::shared_ptr<ImageFace>  getImageFace(const std::string &faceToken);
  std::map<std::string, std::shared_ptr<ImageFace> > getImageFaces() {
    return imageFaces;
  } 

 private:
  std::string userId_;
  std::string userName_;
  std::map<std::string, std::shared_ptr<ImageFace>> imageFaces;
};

class GroupFace {
 public:
  GroupFace(const std::string &groupId);
  int addUserFace(const std::string &userId, const std::string &userName);
  int delUserFace(const std::string &userId);
  std::shared_ptr<UserFace> getUserFace(const std::string &userId);
  std::map<std::string, std::shared_ptr<UserFace>> getUserFaces() {
    return userFaces;
  }
  
 private:
  std::string groupId_;
  std::map<std::string, std::shared_ptr<UserFace>> userFaces;
};

class AppFace {
 public:
  explicit AppFace(const std::string &name);
  int addGroupFace(const std::string &groupId);
  int delGroupFace(const std::string &groupId);
  std::shared_ptr<GroupFace> getGroupFace(const std::string &groupId);
  std::map<std::string, std::shared_ptr<GroupFace> > getGroupFaces() {
    return groupFaces;
  }

 private:
  std::string appName;
  std::map<std::string, std::shared_ptr<GroupFace> > groupFaces;
};

class FaceAgent {
 public:
  static FaceAgent& getFaceAgent();
  int addAppFace(const std::string &appName);
  std::shared_ptr<AppFace> getAppFace(const std::string &appName);
  void getUserFaces(const std::string &appName,
      const std::string &groupName,
      const std::string &userName,
      std::map<std::string, std::shared_ptr<ImageFace>> &faceMap);
  void getDefaultPersonFaces(std::list<PersonFace> &faces);
  int addPersonFace(const PersonFace &face);
  std::map<std::string, std::shared_ptr<AppFace>> getAppFaces() {
    return appFaces;
  }

  private:
    std::map<std::string, std::shared_ptr<AppFace>> appFaces;
};



}
#endif
