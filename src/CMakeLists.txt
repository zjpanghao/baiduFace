
add_definitions(-std=c++11 -g)
set(BASE /home/test/github/kunlib)
set(PDIR /home/test/bai2/college)
include_directories(${PDIR}/include ${BASE}/include   /usr/local/include/zdb ${BASE}/user)
link_directories(./lib ${PDIR}/facelib  ${BASE}/libs ${PDIR}/lib3/json/lib ${PDIR}/lib3/json/lib-ubuntu    ${PDIR}/lib3/ffmpeg-linux/lib  ${PDIR}/lib3/lib64 ${PDIR}/lib3/openblas-linux/lib  ${PDIR}/lib3/opencv-linux/lib /usr/local/lib64 /usr/local/lib)
add_executable(feden featureBufferMemory.cpp main.cpp faceControl.cpp cv_help.cpp httpUtil.cpp userControl.cpp http-server.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepoSql.cpp faceService.cpp)

target_link_libraries(feden pbase64  kunconfig FaceSDK BaiduFaceApi-ubuntu  jsoncpp   event  opencv_imgcodecs opencv_core opencv_imgproc   openblas glog dbpool zdb pthread)

