#ifndef VIDEO_HPP
#define VIDEO_HPP

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <memory>
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

int write_result_video(Video &video, std::string config_filepath);
void run_video_writer_thread(std::shared_ptr<std::vector<Video>> sp_buf);

#endif