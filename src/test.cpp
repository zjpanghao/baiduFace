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
#include "faceRepo.h"
#include "faceService.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <list>
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
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(fname, out_buf);
  std::string base64 =  ImageBase64::encode((const unsigned char*)&out_buf[0], buf_len);
  //std::vector<unsigned char> data(&out_buf[0], &out_buf[0] + buf_len);
  FaceService &service = FaceService::getFaceService();
  LOG(INFO) << service.init();
  std::string faceToken = "aaa";
  int rc = service.addUserFace("227", uid, "zhangsan", base64, faceToken);
  LOG(INFO) << "add userFace " << fname << " rc:" << rc;
}

int test_search(std::string name) {
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(name.c_str(), out_buf);
  std::vector<unsigned char> data(&out_buf[0], &out_buf[0] + buf_len);
  LOG(INFO) << buf_len << "data:" << data.size();
  FaceService &service = FaceService::getFaceService();
  LOG(INFO) << service.init();
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
  rc =  service.search(groups, fResult.faceToken, 2,  searchResult);
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
  LOG(INFO) << service.init();
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
  std::list<std::string> files;
  names.push_back(root);
  while (!names.empty()) {
    std::string current = names.front();
    LOG(INFO) << "current is " << current;
    id++;
    std::stringstream index;
    index << id;
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
        std::string inx = index.str();
        add_image(child.c_str(), inx);
      }
      ent = readdir(dir);
    }
  }
}

void test_attr(const char *fname) {
  FaceService &service = FaceService::getFaceService();
  if (0 !=service.init()) {
    return;
  }
  std::string out_buf;
  int buf_len = ImageBuf::get_buf(fname, out_buf);
  #if 0
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
  initGlog(argv[0]);
//  add_dir("221");
//  test_search(argv[1]);
  FaceService &service = FaceService::getFaceService();
  if (0 !=service.init()) {
    return -1;
  }
  //test_delUser("227", "56");
  //test_quality("33.jpg");
  ev_server_start_multhread(10556, 1); 
  //ev_server_start(10556);
  //test_attr("33.jpg");
  //add_image("33.jpg", "1");
  //test_search("3030.jpg");
  while (1) {
    sleep(10000);
  }
  return 0;
}
