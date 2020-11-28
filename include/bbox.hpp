#ifndef BBOX_HPP
#define BBOX_HPP

#include "opencv2/core/mat.hpp"
#include "opencv2/core/core.hpp"
#include "BoundingBox.h"
#include <vector>
#include <string>

class BBox
{
    public:
    int x; // Bounding box left edge
    int y; // Bounding box top edge?
    int w; // Box width
    int h; // Box height
    int class_id; // Numerical class ID
    std::string class_name; // Class name

    // Constructor function
    BBox();
    BBox(int x, int y, int w, int h, int class_id, std::string &class_name);
    BBox(int x, int y, int w, int h, int class_id, std::vector<std::string> &class_names);
    BBox(tk::dnn::box &box_in, std::string &class_name);
    BBox(tk::dnn::box &box_in, std::vector<std::string> &class_names);
};


// Draw boxes on frame
void get_box_colors(std::vector<cv::Scalar> &colors, int num_classes);
void draw_box_on_frame(cv::Mat &frame, BBox &bounding_box);
void draw_box_on_frame(cv::Mat &frame, BBox &bounding_box, const cv::Scalar &color);
void draw_boxes_on_frame(cv::Mat &frame, std::vector<BBox> &bounding_boxes, std::vector<cv::Scalar> &colors);

#endif