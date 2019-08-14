#include <iostream>
#include <string>
#include <stdio.h>
#include <regex>
#include <sstream>
#include <fstream>
#include <iterator>
#include <thread>
#include "opencv2/opencv.hpp"
#include "baidu_face_api.h"
#include "image_base64.h"
#include "cv_help.h"
#include "json/json.h"
#include "liveness.h"
#include "compare.h"
#include "setting.h"
#include <chrono>
#include "image_buf.h"
#include "glog/logging.h"
#include "faceAgent.h"
#include "faceService.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config/config.h"
#include <list>
#include <chrono>
using namespace std;
using namespace kface;

static void initGlog(const std::string &name) {
  DIR *dir = opendir("log");
  if (!dir) {
    mkdir("log", S_IRWXU);
  } else {
    closedir(dir);
  }
  google::InitGoogleLogging(name.c_str());
  google::SetLogDestination(google::INFO,std::string("log/"+ name + "info").c_str());
  google::SetLogDestination(google::WARNING,std::string("log/"+ name + "warn").c_str());
  google::SetLogDestination(google::GLOG_ERROR,std::string("log/"+ name + "error").c_str());
  FLAGS_logbufsecs = 0;
}

void add_images(std::list<std::string> names) {
  for (std::string &name : names) {
    LOG(INFO) << name;
    //add_image(name.c_str();
  }
}

void add_image(const char *fname, std::string uid) {
  std::string fileName(fname);
  std::regex re("/");
  std::vector<std::string> nameList(std::sregex_token_iterator(fileName.begin(), fileName.end(), re, -1),
      std::sregex_token_iterator());
  if (nameList.size() < 2) {
    return;
  }

  std::string out_buf;
  int buf_len = ImageBuf::get_buf(fname, out_buf);
  std::string base64 =  ImageBase64::encode((const unsigned char*)&out_buf[0], buf_len);
  //std::vector<unsigned char> data(&out_buf[0], &out_buf[0] + buf_len);
  FaceService &service = FaceService::getFaceService();
  std::string faceToken;
  int rc = service.addUserFace("221", uid, nameList[nameList.size() - 1], base64, faceToken);
  if (rc != 0) {
    LOG(INFO) << "add userFace " << fname << " rc:" << rc;
  }
}

int test_search(std::string name) {
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(name.c_str(), out_buf);
  std::vector<unsigned char> data(&out_buf[0], &out_buf[0] + buf_len);
  LOG(INFO) << buf_len << "data:" << data.size();
  FaceService &service = FaceService::getFaceService();
  std::vector<FaceDetectResult> result;
  int rc = service.detect(data, 1, result);
  LOG(INFO) << rc << "size:" << result.size();
  if (rc != 0 || result.size() <= 0) {
    LOG(INFO) << "NO face";
    return -1;
  }
  FaceDetectResult fResult = result[0];
  LOG(INFO) << "token:" << fResult.faceToken;
  std::regex re(",");
  std::string gid = "227,white";
  std::vector<FaceSearchResult> searchResult;
  std::set<std::string> groups(std::sregex_token_iterator(gid.begin(), gid.end(), re, -1),
  std::sregex_token_iterator());
  auto timeNow = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
  rc =  service.search(groups, fResult.faceToken, 2,  searchResult);
  auto time2 = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
  LOG(INFO) << "now:" << timeNow.count() << " after:" << time2.count();
  LOG(INFO) << rc << " size:" << searchResult.size();
  for (FaceSearchResult result : searchResult) {
  LOG(INFO) << rc 
    << "userId:" << result.userId <<" " << result.score;
  }
  return 0;
}

int test_detect(const char *fname) {
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(fname, out_buf);
  std::vector<unsigned char> data(&out_buf[0], &out_buf[0] + buf_len);
  LOG(INFO) << buf_len << "data:" << data.size();
  FaceService &service = FaceService::getFaceService();
  std::vector<FaceDetectResult> result;
  int rc = service.detect(data, 1, result);
  LOG(INFO) << rc << "size:" << result.size();
  FaceDetectResult fResult = result[0];
  LOG(INFO) << fResult.faceToken;
  return 0;
}

void add_dir(std::string root) {
  DIR *dir = NULL;
  struct dirent *ent = NULL;
  struct stat st;
  int id = 50;
  std::string name;
  std::string dname;
  std::list<std::string> names;
  names.push_back(root);
  while (!names.empty()) {
    std::string current = names.front();
    LOG(INFO) << "current is " << current;
    names.pop_front();
    dir = opendir(current.c_str());
    if (!dir) {
      LOG(ERROR) << "no dir:" << current;
      continue;
    }
    ent = readdir(dir);
    while (ent != NULL) {
      LOG(INFO) << ent->d_name;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
        ent = readdir(dir);
        continue;
      }
      std::stringstream ss;
      ss << current << "/" << ent->d_name;
      std::string child = ss.str();
      memset(&st, 0, sizeof(st));
      stat(child.c_str(), &st);
      if (S_ISDIR(st.st_mode)) {
        LOG(INFO) << "push" << ss.str();
        names.push_back(child);
      } else if (S_ISREG(st.st_mode)) {
        LOG(INFO) << "add " << child;
        std::stringstream index;
        index << id++;
        std::string inx = index.str();
        add_image(child.c_str(), inx);
      }
      ent = readdir(dir);
    }
  }
}

void add_dir2(std::string root) {
  DIR *dir = NULL;
  struct dirent *ent = NULL;
  struct stat st;
  int id = 50;
  std::string name;
  std::string dname;
  std::list<std::string> names;
  names.push_back(root);
  while (!names.empty()) {
    std::string current = names.front();
    LOG(INFO) << "current is " << current;
    names.pop_front();
    dir = opendir(current.c_str());
    if (!dir) {
      LOG(ERROR) << "no dir:" << current;
      continue;
    }
    ent = readdir(dir);
    while (ent != NULL) {
      LOG(INFO) << ent->d_name;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
        ent = readdir(dir);
        continue;
      }
      std::stringstream ss;
      ss << current << "/" << ent->d_name;
      std::string child = ss.str();
      memset(&st, 0, sizeof(st));
      stat(child.c_str(), &st);
      if (S_ISDIR(st.st_mode)) {
        LOG(INFO) << "push" << ss.str();
        names.push_back(child);
      } else if (S_ISREG(st.st_mode)) {
        LOG(INFO) << "add " << child;
        std::stringstream index;
        index << id++;
        std::string inx = index.str();
        add_image(child.c_str(), inx);
      }
      ent = readdir(dir);
    }
  }
}


void test_attr(const char *fname) {
  FaceService &service = FaceService::getFaceService();
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(fname, out_buf);
  #if 1
  auto ft =  service.getAttr((const unsigned char*)&out_buf[0], buf_len);
  if (ft != nullptr) {
    printf("%d\n", ft->age);
    printf("%d\n", ft->gender);
    printf("%.2f\n", ft->genderConfidence);
  }
  #endif
}

void test_quality(const char *fname) {
#if 0
  FaceService &service = FaceService::getFaceService();
  if (0 !=service.init()) {
    return;
  }
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(fname, out_buf);
  auto ft =  service.faceQuality((const unsigned char*)&out_buf[0], buf_len);
  if (ft != nullptr) {
  }
 #endif
}

void test_delUser(std::string gid, std::string uid) {
  FaceService &service = FaceService::getFaceService();
  int rc = service.delUser(gid, uid);
  LOG(INFO) << rc; 
}

void ev_server_start(int);
void ev_server_start_multhread(int, int);
int main(int argc, char *argv[]) {
  kunyan::Config config("config.ini");
  std::string portConfig = config.get("server", "port");
  std::stringstream ss;
  ss << portConfig;
  int port;
  ss >> port;
  std::string name(argv[0]);
  initGlog(name);
  FaceService &service = FaceService::getFaceService();
  ss.clear();
  ss.str("");

  // mongo
  ss.clear();
  ss.str("");
  //const char *uri_string = "mongodb://test:123456@192.168.1.111:27017/test";
  std::shared_ptr<DBPool> pool = std::make_shared<DBPool> ();
  if (pool->PoolInit(new DataSource(config)) < 0) {
    LOG(ERROR) << "create db  pool error";
    return -1;
  }

  ss << config.get("redis", "num");
  int num = 1;
  ss >> num;

  ss.clear();
  ss.str("");
  ss << config.get("redis", "max");
  int max = 10;
  ss >> max;

  int redisPort = 0 ;
  ss.clear();
  ss.str("");
  ss << config.get("redis", "port");
  ss >> redisPort;
  
  std::shared_ptr<RedisPool> redisPool(
  new RedisPool(config.get("redis", "ip"), 
                redisPort, num, max, "3",
                config.get("redis", "password")));
  
  if (0 !=service.init(pool, redisPool, "", true)) {
    LOG(ERROR) << "server init error";
    return -1;
  }
#if 0
  
  if (0 !=service.init(pool, dbName, false)) {
    return -1;
  }
  //test_delUser("227", "227");
  //test_quality("33.jpg");
  //ev_server_start_multhread(10556, 1); 
  //ev_server_start(10556);
  //test_attr("33.jpg");
  
  std::list<PersonFace> faces;
  //localLoadPersonFaces("faces.db", faces);
  for (PersonFace &face : faces) {
     if (0 != kface::repoAddUserFace(face)) {
      LOG(ERROR) << "repo add userface error" << face.userName;
      return -9;
    }
  }
#endif
  add_dir2("/home/panghao/18");
  //test_delUser("227", "1");
  //add_image("33.jpg", "1");
  //add_image("/home/panghao/222/yuxiang/3743.png", "3121");
  //add_image("panghao.jpg", "1");
  //test_search("33.jpg");
  //test_search("3030.jpg");
  //while (1) {
  //  sleep(10000);
  //}
  return 0;
}
