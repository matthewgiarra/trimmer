#ifndef DRAW_HPP
#define DRAW_HPP

#include "BoundingBox.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/core/core.hpp"
#include <vector>


void get_box_colors(std::vector<cv::Scalar> &colors, int num_classes);
void draw_box_on_frame(cv::Mat frame, BBox bounding_box);
void draw_box_on_frame(cv::Mat frame, BBox bounding_box, cv::Scalar color);
void draw_boxes_on_frame(cv::Mat frame, std::vector<BBox> bounding_boxes, std::vector<cv::Scalar> colors);



#endif