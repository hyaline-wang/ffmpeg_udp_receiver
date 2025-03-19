cmake_minimum_required(VERSION 3.10) # 设置 CMake 的最低版本要求

project(decode_video) # 设置项目名称

# 设置 C++ 编译标准 (可选，但推荐)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenCV_DIR /home/hao/ffmepeg_udp_decode/opencv-4.5.5/build)

include(FindPkgConfig)

pkg_check_modules(FFmpeg REQUIRED libavformat libavcodec libavutil libswscale)  # Find FFmpeg
# # 查找 FFmpeg
# find_package(FFmpeg REQUIRED COMPONENTS avformat avcodec avutil swscale)
# if(NOT FFmpeg_FOUND)
#     message(FATAL_ERROR "FFmpeg not found. Please install FFmpeg development libraries.")
# endif()

# 查找 OpenCV
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV development libraries.")
endif()

# 添加可执行文件
add_executable(decode_video decode_video.cpp)

# 链接 FFmpeg 库
target_link_libraries(decode_video PRIVATE ${FFmpeg_LIBRARIES})

# 链接 OpenCV 库
target_link_libraries(decode_video PRIVATE ${OpenCV_LIBS})


add_executable(decode_video_udp decode_videoudp.cpp)
# 链接 FFmpeg 库
target_link_libraries(decode_video_udp PRIVATE ${FFmpeg_LIBRARIES})

# 链接 OpenCV 库
target_link_libraries(decode_video_udp  PRIVATE ${OpenCV_LIBS})



# (可选) 设置安装规则
install(TARGETS decode_video DESTINATION bin)