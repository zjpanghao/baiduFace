#include "faceAgent.h"
#include "faceRepo.h"
#include <glog/logging.h>
namespace kface {

UserFace::UserFace(const std::string &userId,
                   const std::string &userName) {
  userId_ = userId;
  userName_ = userName;
}

int UserFace::addImageFace(std::shared_ptr <ImageFace> imageFace) {
  if (imageFaces.size() > 5) {
    return -1;
  }
  if (imageFaces.count(imageFace->faceToken) != 0) {
    LOG(INFO) << "add face twice for:" << imageFace->faceToken;
    return -2;
  }
  imageFaces.insert(std::make_pair(imageFace->faceToken, imageFace));
  return 0;
}

int UserFace::delImageFace(const std::string &faceToken) {
   imageFaces.erase(faceToken);
   return 0;
}

std::shared_ptr<ImageFace>  UserFace::getImageFace(const std::string &faceToken) {
  auto it = imageFaces.find(faceToken);
  if (it == imageFaces.end()) {
    return nullptr;
  }
  return it->second;
}

GroupFace::GroupFace(const std::string &groupId) {
  groupId_ = groupId; 
}

std::shared_ptr<UserFace> GroupFace::getUserFace(const std::string &userId){
  auto it = userFaces.find(userId);
  if (it == userFaces.end()) {
    return nullptr;
  }
  return it->second;
}

int GroupFace::addUser(const std::string &userId, const std::string &userName) {
  if (userId == "") {
    return -1;
  }
  if (userFaces.count(userId) != 0) {
    return -2;
  }
  std::shared_ptr<UserFace> userFace(new UserFace(userId, userName));
  userFaces.insert(std::make_pair(userId, userFace));
  return 0;
}

int GroupFace::delUser(const std::string &userId) {
  if (userId == "") {
    return -1;
  }
  userFaces.erase(userId);
  return 0;
}

int AppFace::addGroupFace(const std::string &groupId) {
  if (0 != groupFaces.count(groupId)) {
    return -1;
  }
  std::shared_ptr<GroupFace> group(new GroupFace(groupId));
  groupFaces.insert(std::make_pair(groupId, group));
  return 0;
}

std::shared_ptr<GroupFace> AppFace::getGroupFace(const std::string &groupId) {
  auto it = groupFaces.find(groupId);
  if (it == groupFaces.end()) {
    return nullptr;
  }
  return it->second;
}

AppFace::AppFace(const std::string &name) {
  appName = name;
}

int FaceAgent::addAppFace(const std::string &appName) {
  if (0 != appFaces.count(appName)) {
    return -1;
  }
  std::shared_ptr<AppFace> app(new AppFace(appName));
  appFaces.insert(std::make_pair(appName, app));
  return 0;
}

std::shared_ptr<AppFace> FaceAgent::getAppFace(const std::string &appName) {
  auto it = appFaces.find(appName);
  if (it == appFaces.end()) {
    return nullptr;
  }
  return it->second;
}

FaceAgent& FaceAgent::getFaceAgent() {
  static FaceAgent faceAgent;
  return faceAgent;
}

void FaceAgent::getUserFaces(const std::string &appName,
    const std::string &groupName,
    const std::string &userName,
    std::map<std::string, std::shared_ptr<ImageFace>> &faceMap) {
  auto app = getAppFace(appName);
  if (app == nullptr) {
    return;
  }
  auto group = app->getGroupFace(groupName);
  if (group != nullptr) {
    auto user = group->getUserFace(userName);
    if (user != nullptr) {
      faceMap = user->getImageFaces();
    }
  }
}

int FaceAgent::delPerson(const PersonFace &face) {
  std::string appName = (face.appName == "" ? DEFAULT_APP_NAME : face.appName);
  auto app = getAppFace(appName);
  if (app == nullptr) {
    return -1;
  }
  auto group = app->getGroupFace(face.groupId);
  if (group == nullptr) {
      return -2;
  }
  
  int rc = group->delUser(face.userId);
  return rc;
}

int FaceAgent::delPersonFace(const PersonFace &face) {
  std::string appName = (face.appName == "" ? DEFAULT_APP_NAME : face.appName);
  auto app = getAppFace(appName);
  if (app == nullptr) {
    return -1;
  }
  auto group = app->getGroupFace(face.groupId);
  if (group == nullptr) {
      return -2;
  }

  auto user = group->getUserFace(face.userId);
  if (user == nullptr) {
      return -3;
  }
  int rc = user->delImageFace(face.image->faceToken);
  return rc;
}

int FaceAgent::addPersonFace(const PersonFace &face) {
  std::string appName = (face.appName == "" ? DEFAULT_APP_NAME : face.appName);
  auto app = getAppFace(appName);
  if (app == nullptr) {
    addAppFace(appName);
    app = getAppFace(appName);
    if (app == nullptr) {
      return -1;
    }
  }
  auto group = app->getGroupFace(face.groupId);
  if (group == nullptr) {
    app->addGroupFace(face.groupId);
    group = app->getGroupFace(face.groupId);
    if (group == nullptr) {
      return -2;
    }
  }

  auto user = group->getUserFace(face.userId);
  if (user == nullptr) {
    group->addUser(face.userId, face.userName);
    user = group->getUserFace(face.userId);
    if (user == nullptr) {
      return -3;
    }
  }

  int rc = user->addImageFace(face.image);
  return rc;
}

void FaceAgent::getDefaultPersonFaces(std::list<PersonFace> &faces) {
  auto app = getAppFace(DEFAULT_APP_NAME);
  if (app == nullptr) {
    return;
  }
  auto groups = app->getGroupFaces();
  for (auto it = groups.begin(); it != groups.end(); it++) {
    auto users = it->second->getUserFaces();
    for (auto uit = users.begin(); uit != users.end(); uit++) {
      auto images = uit->second->getImageFaces();
      for (auto iit = images.begin(); iit != images.end(); iit++) {
        PersonFace face;
        face.appName = DEFAULT_APP_NAME;
        face.groupId = it->first;
        face.userId = uit->first;
        face.userName = uit->second->getUserName();
        face.image = iit->second;
        faces.push_back(face);
      }
    }
  }
}

}
