#ifndef INCLUDE_FACE_AGENT_H
#define INCLUDE_FACE_AGENT_H
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include "faceEntity.h"
#include <pthread.h>
#include <unistd.h>

#define DEFAULT_APP_NAME "door"

namespace kface {
struct PersonFace;

class UserFace {
 public:
  UserFace(const std::string &userId, 
           	   const std::string &userName);
  int addImageFace(std::shared_ptr <ImageFace> imageFace);
  int delImageFace(const std::string &faceToken);
  std::shared_ptr<ImageFace>  getImageFace(const std::string &faceToken);
  std::string getUserName() {
    return userName_;
  }
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
  int addUser(const std::string &userId, const std::string &userName);
  int delUser(const std::string &userId);
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

class LockMethod {
  public:
    virtual void lock(pthread_rwlock_t *lock) = 0;
};

class RLockMethod : public LockMethod {
  void lock(pthread_rwlock_t *lock) override {
    pthread_rwlock_rdlock(lock);
  }
};

class WLockMethod : public LockMethod {
  void lock(pthread_rwlock_t *lock) override {
    pthread_rwlock_wrlock(lock);
  }
};

class RWLockGuard {
  public:
    RWLockGuard(LockMethod &method, pthread_rwlock_t *lock):lock_(lock) {
      method.lock(lock);
    }

    ~RWLockGuard() {
      pthread_rwlock_unlock(lock_);
    }

  private:
    pthread_rwlock_t *lock_;
    bool locked_{false};
};

class FaceAgent {
 public:
  static FaceAgent& getFaceAgent();
  void getDefaultPersonFaces(std::list<PersonFace> &faces);
  int addPersonFace(const PersonFace &face);
  int delPersonFace(const PersonFace &face);
  int delPerson(const PersonFace &face);

  private:
   FaceAgent() {
     pthread_rwlock_init(&lock_, NULL);
   }

   void getUserFaces(const std::string &appName,
                     const std::string &groupName,
                     const std::string &userName,
                     std::map<std::string, std::shared_ptr<ImageFace>> &faceMap);
   std::shared_ptr<AppFace> getAppFace(const std::string &appName);
   std::map<std::string, std::shared_ptr<AppFace>> getAppFaces() {
     return appFaces;
   }
   int addAppFace(const std::string &appName);
   std::map<std::string, std::shared_ptr<AppFace>> appFaces;
   pthread_rwlock_t lock_; 
};

}
#endif
