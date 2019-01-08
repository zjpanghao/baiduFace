include_directories(../include)
add_definitions(-std=c++11 -g)
set(BASE /home/panghao/kunlib)
link_directories(${BASE}/libs ../../lib3/json/lib ../../lib3/ffmpeg-linux/lib ../../lib ../../lib3/lib64 ../../lib3/openblas-linux/lib  ../../lib3/opencv-linux/lib)
add_executable(feden main.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepo.cpp faceService.cpp)
add_executable(feden_test test.cpp http-server.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepo.cpp faceService.cpp)
target_link_libraries(feden FaceSDK BaiduFaceApi-centos json  opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio avformat avcodec avutil swscale swresample openblas jpeg  tiff png12 glog pthread)
target_link_libraries(feden_test FaceSDK BaiduFaceApi-centos event jsoncpp  opencv_imgcodecs opencv_core opencv_imgproc opencv_highgui opencv_video opencv_videoio avformat avcodec avutil swscale swresample openblas jpeg  tiff png12 glog pthread)

