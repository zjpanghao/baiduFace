# baiduFace
基于baidu人脸识别sdk的一个服务封装
目前支持人脸的检测和1：N的识别，人脸库可以
选择mysql或mongo 存储，网络框架基于libevent和libevhtp
除了用到的开源库之外，还需要kunlib库， 这个库主要包含了操纵数据库，网络框架以及一些常用算法的封装。

