#ifndef FRAME_HPP
#define FRAME_HPP

#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"
#include "BoundingBox.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <zmq.hpp>

class Frame
{
    public:
    cv::Mat image;

    // Bounding boxes
    std::vector<tk::dnn::box> detection_boxes;
    
    // Image info (header)
    nlohmann::json header;

    // Constructor
    Frame();
    Frame(zmq::message_t &recv_msg);
    void populate(zmq::message_t &recv_msg);

};

#endif