
add_definitions(-std=c++11)
set(BASE /home/panghao/kunlib)
include_directories(../include ${BASE}/include  ${BASE}/include/hiredis ${BASE}/user)
link_directories(/usr/local/lib64 ${BASE}/libs ${BASE}/libs/opencv)
add_executable(feden main.cpp faceControl.cpp cv_help.cpp httpUtil.cpp userControl.cpp http-server.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepo.cpp faceService.cpp)
#add_executable(feden_test test.cpp cv_help.cpp httpUtil.cpp faceControl.cpp userControl.cpp http-server.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepo.cpp faceService.cpp)
#add_executable(test cv_help.cpp liveness.cpp image_base64.cpp image_buf.cpp setting.cpp test_face.cpp compare.cpp)
#target_link_libraries(test mongoc-1.0 bson-1.0 kunconfig FaceSDK BaiduFaceApi-centos json event  opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio avformat avcodec avutil swscale swresample openblas jpeg  tiff png12 glog pthread)
target_link_libraries(feden kunApiPool  openblas kunFace dlib  mongoc-1.0 bson-1.0 kunconfig hiredis  jsoncpp  event  opencv_dnn opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio glog pthread)
#target_link_libraries(feden_test hiredis mongoc-1.0 bson-1.0 kunconfig FaceSDK BaiduFaceApi-centos event jsoncpp  opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio avformat avcodec avutil swscale swresample openblas jpeg  tiff png12 glog pthread)

