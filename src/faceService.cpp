#include "faceService.h"
#include "image_base64.h"
#include "md5.h"
#include "faceAgent.h"
#include "personFaceRepo.h"
#define MAX_FACE_TRACK 5
using namespace cv;
namespace kface {
FaceService& FaceService::getFaceService() {
  static FaceService faceService;
  return faceService;
}

FaceService::FaceService() {
}

int FaceService::initAgent() {
  FaceAgent faceAgent = FaceAgent::getFaceAgent();
  std::list<PersonFace> faces;
  loadPersonFaces(faces);
  for (PersonFace &face : faces) {
    faceAgent.addPersonFace(face);
  }
  return 0;
}

int FaceService::init() {
  api_.reset(new BaiduFaceApi());
  int rc = api_->sdk_init(false);
  if (rc != 0) {
    return rc;
  }
  if (!api_->is_auth()) {
    return -1;
  }
  api_->set_min_face_size(15);

  initAgent();
  return 0;
}

/*
 * only detect one person
 */
int FaceService::detect(const std::vector<unsigned char> &data,
    int faceNum,
    std::vector<FaceDetectResult> &detectResult 
    ) {
  int rc = 0;
  Mat m = imdecode(Mat(data), 1);
  const float *feature = nullptr;
  int count = api_->get_face_feature_by_buf(&data[0], data.size(), feature);
  if (count != 512) {
    return -1;
  }
  std::vector<TrackFaceInfo>  *out = new std::vector<TrackFaceInfo>();
  int nFace = api_->track_max_face_by_buf(out, &data[0], data.size());
  if (nFace == 0) {
    return -2;
  }
  for (TrackFaceInfo &info : *out) {
    FaceDetectResult result;
    result.trackInfo = info;
    result.faceToken = MD5(ImageBase64::encode(&data[0], data.size())).toStr();
    FaceBuffer buffer;
    buffer.feature.assign(feature, feature + 512);
    faceBuffers.insert(std::make_pair(result.faceToken, buffer)); 
    detectResult.push_back(result);
  } 

  return rc;
}

}
