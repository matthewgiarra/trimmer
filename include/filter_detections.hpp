
#ifndef FILTER_DETECTIONS_HPP
#define FILTER_DETECTIONS_HPP

#include "opencv2/core/core.hpp"
#include "BoundingBox.h"

// Functions for filtering detections based on various criteria
float boxesOverlap(const float p1, const float d1, const float p2, const float d2);

float boxesIntersection(const tk::dnn::box &a, const tk::dnn::box &b);

float max_possible_intersection(const tk::dnn::box &a, const tk::dnn::box &b);

float IoMax(const tk::dnn::box &a, const tk::dnn::box &b); // intersection over max possible intersection

bool filter_detection_by_IoMax(const tk::dnn::box &detection, const tk::dnn::box &roi, const float roi_threshold);

bool filter_detection_by_class(const tk::dnn::box &detection, const std::vector<int> &trimmer_class_nums);

bool any_detections(const std::vector<tk::dnn::box> &detection_boxes, const std::vector<int> &trimmer_class_nums);

void get_valid_detections(std::vector<tk::dnn::box> &valid_detections, const std::vector<tk::dnn::box> &all_detections, const std::vector<int> &trimmer_class_nums);

void get_valid_detections(std::vector<tk::dnn::box> &valid_detections, const std::vector<tk::dnn::box> &all_detections, const std::vector<int> &trimmer_class_nums, const tk::dnn::box &roi, const float roi_threshold);

#endif