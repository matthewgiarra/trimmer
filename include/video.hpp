#ifndef VIDEO_HPP
#define VIDEO_HPP


// There is some compile error related to JSON 
// where I have to include an openCV library
// before including BoundingBox.h
//(core/core.hpp, core/mat.hpp both work)
// The only way I can get stuff to compile
// is to first include opencv anywhere I include BoundingBox.h  
#include "opencv2/core/core.hpp"
#include "BoundingBox.h" 
#include "draw.hpp"
#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

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

int write_result_video(Video &video, const std::string &config_filepath, const std::vector<int> &trimmer_class_nums, const std::vector<std::string> &class_names);

void run_video_writer_thread(std::shared_ptr<std::timed_mutex> video_queue_mutex_sp, std::shared_ptr<std::queue<Video>> video_queue_buf_sp, const std::string &config_filepath, const std::vector<int> &trimmer_class_nums, const std::vector<std::string> &class_names);

#endif