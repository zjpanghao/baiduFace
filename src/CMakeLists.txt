add_definitions(-std=c++11 -g)
set(BASE /home/test/github/kunlib)
set(PDIR /home/test/baidu/college)
message(STATUS ${CMAKE_SKIP_RPATH})
include_directories(${PDIR}/include ${BASE}/include  ~/pg/redis-5.0.4/deps/hiredis  /usr/local/include/zdb ${BASE}/user /home/test/opencv3)
link_directories(./lib /home/test/pg/redis-5.0.4/deps/hiredis ${PDIR}/facelib  ${BASE}/libs ${PDIR}/lib3/json/lib ${PDIR}/lib3/json/lib-ubuntu    ${PDIR}/lib3/ffmpeg-linux/lib  ${PDIR}/lib3/lib64 ${PDIR}/lib3/openblas-linux/lib  ${PDIR}/lib3/opencv-linux/lib /usr/local/lib64 /usr/local/lib)
add_executable(feden featureBufferRedis.cpp featureBufferMemory.cpp main.cpp faceControl.cpp cv_help.cpp httpUtil.cpp userControl.cpp http-server.cpp image_buf.cpp image_base64.cpp md5.cpp faceAgent.cpp faceRepoSql.cpp faceService.cpp)

target_link_libraries(feden predis hiredis pbase64  kunconfig FaceSDK BaiduFaceApi-ubuntu  jsoncpp   event  opencv_imgcodecs opencv_core opencv_imgproc   openblas glog dbpool zdb pthread)
 

