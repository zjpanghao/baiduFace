
add_definitions(-std=c++11 -g)
set(BASE /home/panghao/github/kunlib)
include_directories(../include ${BASE}/include ${BASE}/include/hiredis  ${BASE}/include/zdb ${BASE}/user)
link_directories(./lib /usr/local/lib64 ${BASE}/libs ../../lib ../../lib3/json/lib  ../../lib3/ffmpeg-linux/lib ../../lib ../../lib3/lib64 ../../lib3/openblas-linux/lib  ../../lib3/opencv-linux/lib)
add_executable(feden featureBufferMemory.cpp main.cpp faceControl.cpp cv_help.cpp httpUtil.cpp userControl.cpp http-server.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepoSql.cpp faceService.cpp)
add_executable(feden_test featureBufferMemory.cpp test.cpp cv_help.cpp httpUtil.cpp faceControl.cpp userControl.cpp http-server.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepoSql.cpp faceService.cpp)
#add_executable(test cv_help.cpp liveness.cpp image_base64.cpp image_buf.cpp setting.cpp test_face.cpp compare.cpp)
#target_link_libraries(test mongoc-1.0 bson-1.0 kunconfig FaceSDK BaiduFaceApi-centos json event  opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio avformat avcodec avutil swscale swresample openblas jpeg  tiff png12 glog pthread)
target_link_libraries(feden pbase64  kunconfig FaceSDK BaiduFaceApi-centos hiredis  jsoncpp  event  opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio avformat avcodec avutil swscale swresample openblas jpeg  tiff png12 glog dbpool zdb pthread)
target_link_libraries(feden_test pbase64 hiredis  kunconfig FaceSDK BaiduFaceApi-centos event jsoncpp  opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio avformat avcodec avutil swscale swresample openblas jpeg  tiff png12 glog dbpool zdb pthread)

