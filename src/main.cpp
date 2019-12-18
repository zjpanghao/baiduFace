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
#include <sys/prctl.h>
#include "config/config.h"

#include "evdrv/evDrvControl.h"
#include "faceControl.h"
#include "userControl.h"
#include "resource/resource.h"

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
  std::stringstream ss;
  initGlog(name);
   std::vector<std::shared_ptr<GeneralControl>> urls{
    std::shared_ptr<kface::FaceControl>(
      new kface::FaceControl()),
    std::shared_ptr<kface::UserControl>(
      new kface::UserControl())
  };
  Resource::getResource().init(config);
  EvDrvControl::startServer(config, urls); 
  while (1) {
    sleep(10000);
  }
  return 0;
}
