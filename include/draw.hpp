#ifndef DRAW_HPP
#define DRAW_HPP

#include "opencv2/core/mat.hpp"
#include "opencv2/core/core.hpp"
#include "BoundingBox.h"
#include <vector>

void get_box_colors(std::vector<cv::Scalar> &colors, int num_classes);
void draw_box_on_frame(cv::Mat &frame, tk::dnn::box &detection_box, const std::vector<std::string> &class_names);
void draw_box_on_frame(cv::Mat &frame, tk::dnn::box &detection_box, const cv::Scalar &color, const std::vector<std::string> &class_names);
void draw_boxes_on_frame(cv::Mat &frame, std::vector<tk::dnn::box> &detection_boxes, const std::vector<cv::Scalar> &colors, const std::vector<std::string> &class_names);

#endif