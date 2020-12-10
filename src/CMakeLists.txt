add_definitions(-DEVHTP_DEBUG=1 -std=c++11 -g)
set(BASE ~/github/kunlib)
set(PDIR ~/test-face-api)
message(STATUS ${CMAKE_SKIP_RPATH})
include_directories(${PDIR}/include ${BASE}/include  /usr/local/include/zdb ${BASE}/user)
link_directories(/${PDIR})
link_directories(~/github/libevhtp/build ./lib /usr/local/lib64  ${PDIR}/facelib  ${BASE}/libs  ${PDIR}/lib3/json/lib-ubuntu    ${PDIR}/lib3/ffmpeg-linux/lib  ${PDIR}/lib3/lib64 ${PDIR}/lib3/openblas-linux/lib  ${PDIR}/lib3/opencv-linux/lib /usr/local/lib64 /usr/local/lib)
add_executable(feden  featureBufferRedis.cpp featureBufferMemory.cpp main.cpp faceControl.cpp cv_help.cpp httpUtil.cpp userControl.cpp  image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepoSql.cpp faceService.cpp)

target_link_libraries(feden jsoncpp resource ktimer evdrv evhtp predis hiredis pbase64  kunconfig FaceSDK BaiduFaceApi-ubuntu     event opencv_highgui  opencv_imgcodecs opencv_core opencv_imgproc   openblas glog dbpool zdb pthread)
 

