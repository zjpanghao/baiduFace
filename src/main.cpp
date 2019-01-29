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
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>
#include "faceAgent.h"
#include "faceRepo.h"
#include "faceService.h"
#include "config/config.h"
#include <mongoc/mongoc.h>


using kface::FaceService;

extern void ev_server_start_multhread(int port, int nThread);

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

int main(int argc, char *argv[]) {
  kunyan::Config config("config.ini");
  std::string portConfig = config.get("server", "port");
  std::stringstream ss;
  ss << portConfig;
  int port;
  ss >> port;
  std::string name(argv[0]);
  daemon(1, 0);
  initGlog(name);
  FaceService &service = FaceService::getFaceService();
  ss.clear();
  ss.str("");
  ss << config.get("redis", "port");
  int redisPort;
  ss >> redisPort;

  ss.clear();
  ss.str("");
  ss << config.get("redis", "num");
  int num = 1;
  ss >> num;

  ss.clear();
  ss.str("");
  ss << config.get("redis", "max");
  int max = 10;
  ss >> max;
  
  std::shared_ptr<RedisPool> redisPool(
  new RedisPool(config.get("redis", "ip"), 
                redisPort, num, max, "3",
                config.get("redis", "password")));
  // mongo
  ss.clear();
  ss.str("");
  ss << config.get("mongo", "uri");
  std::string uriString(ss.str());
 
  ss.clear();
  ss.str("");
  ss << config.get("mongo", "db");
  std::string dbName(ss.str());
  //const char *uri_string = "mongodb://test:123456@192.168.1.111:27017/test";
  bson_error_t error;
  mongoc_uri_t *uri = mongoc_uri_new_with_error(uriString.c_str(), &error);
  if (!uri) {
     LOG(ERROR) << "create mongo poll error" << error.message;
     return -1;
  }
  mongoc_client_pool_t *pool = mongoc_client_pool_new(uri);
  if (!pool) {
    LOG(ERROR) << "create mongo poll error";
    return -1;
  }
  
  if (0 !=service.init(pool, dbName)) {
    return -1;
  }
  
  ev_server_start_multhread(port, 1); 
  while (1) {
    sleep(10000);
  }
  return 0;
}
