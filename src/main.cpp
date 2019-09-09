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
#include "faceRepoSql.h"
#include "faceService.h"
#include "config/config.h"
#include "datasource/dataSource.h"
#include "db/dbpool.h"
#include "featureBufferMemory.h"


using kface::FaceService;
using kface::FeatureBuffer;
using kface::FeatureBufferMemory;
extern void ev_server_start_multhread(const char *ip, int port, int nThread);

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
  std::string name(argv[0]);
  if (argc == 2 && strcmp(argv[1], "-d") == 0) {
    daemon(1, 0);
  }
  kunyan::Config config("config.ini");
  std::string ip;
  std::string portConfig = config.get("server", "port");
  std::stringstream ss;
  ss << portConfig;
  int port;
  ss >> port;

  ip = config.get("server", "ip");
  initGlog(name);
  FaceService &service = FaceService::getFaceService();
  DataSource dataSource(config);
  std::shared_ptr<DBPool> pool = std::make_shared<DBPool> ();
  if (pool->PoolInit(&dataSource) < 0) {
    LOG(ERROR) << "create db  pool error";
    return -1;
  }
  std::shared_ptr<FeatureBuffer> featureBuffer =
    std::make_shared<FeatureBufferMemory> ();
  if (0 !=service.init(pool, featureBuffer, config)) {
    LOG(ERROR) << "init error";
    return -1;
  }
  
  ev_server_start_multhread(ip.c_str(), port, 1); 
  while (1) {
    sleep(10000);
  }
  return 0;
}
