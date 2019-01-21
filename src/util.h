#ifndef INCLUDE_RW_LOCK_H
#define INCLUDE_RW_LOCK_H
#include <glog/logging.h>
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
#endif
