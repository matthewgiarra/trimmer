#ifndef VIDEO_HPP
#define VIDEO_HPP

#include <iostream>
#include <queue>
#include <string>
#include <memory>
#include <mutex>
#include "boost/filesystem.hpp"

class Video
{  
    public:
    bool finished;
    std::string path;
    std::queue<int> detection_framenums;

    // Member functions
    Video();
    Video(std::string &video_path);
    Video(boost::filesystem::path &video_path);
};

int write_result_video(Video &video, const std::string &config_filepath);
void run_video_writer_thread(std::shared_ptr<std::timed_mutex> video_queue_mutex_sp, std::shared_ptr<std::queue<Video>> video_queue_buf_sp, const std::string &config_filepath);

#endif