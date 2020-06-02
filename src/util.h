#ifndef INCLUDE_RW_LOCK_H
#define INCLUDE_RW_LOCK_H
#include <glog/logging.h>
#include <json/json.h>
class LockMethod {
  public:
    virtual void lock(pthread_rwlock_t *lock) = 0;
    virtual ~LockMethod() {
    }
};

class RLockMethod : public LockMethod {
  public:
  ~RLockMethod() {
    LOG(INFO) << "Release rlock";
  }
  void lock(pthread_rwlock_t *lock) override {
    pthread_rwlock_rdlock(lock);
  }
};

class WLockMethod : public LockMethod {
  public:
  ~WLockMethod() {
    LOG(INFO) << "Release wlock";
  }
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
};


//template<class E>
namespace baidu {
class JsonUtil {
  public:
    template <typename T, class = typename std::enable_if<!std::is_same<T, std::string>::value>::type>
    static void getJsonValue(const Json::Value &value, const std::string &key, T &t) {
      if (value.isNull() 
        || !value.isMember(key) 
        || !value[key].isString()) {
        return;
      }
      std::stringstream ss;
      ss <<value[key].asString();
      ss >> t;
    }

    template <typename T, class = typename std::enable_if<std::is_same<T, std::string>::value>::type>
    static void getJsonStringValue(const Json::Value &value, const std::string &key, T &t) {
      if (value.isNull() 
        || !value.isMember(key) 
        || !value[key].isString()) {
        return;
      }
      t = value[key].asString();
    }
};
}

#endif
