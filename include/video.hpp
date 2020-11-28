#ifndef VIDEO_HPP
#define VIDEO_HPP

#include "bbox.hpp"
#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include "BoundingBox.h"

class Video
{  
    public:
    bool finished;
    std::string path;
    std::queue<int> detection_framenums;
    std::queue<std::vector<tk::dnn::box>> detection_boxes;

    // Member functions
    Video();
    Video(const std::string &video_path);
};

int write_result_video(Video &video, const std::string &config_filepath);
void run_video_writer_thread(std::shared_ptr<std::timed_mutex> video_queue_mutex_sp, std::shared_ptr<std::queue<Video>> video_queue_buf_sp, const std::string &config_filepath, const std::vector<int> &trimmer_class_nums);
bool any_detections(const std::vector<tk::dnn::box> &detection_boxes, const std::vector<int> &trimmer_class_nums);

#endif