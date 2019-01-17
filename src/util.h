#ifndef INCLUDE_RW_LOCK_H
#define INCLUDE_RW_LOCK_H
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
#endif
