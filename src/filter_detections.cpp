
#include "filter_detections.hpp"
#include "opencv2/core/core.hpp"
#include "BoundingBox.h"

// This file just reproduces some member functions from tkDNN/src/tk::dnn::box.cpp
// the functions don't seem to be defined in any of tkDNN's headers.

float boxesOverlap(const float p1, const float d1, const float p2, const float d2)
{
    float l1 = p1;
    float l2 = p2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = p1 + d1;
    float r2 = p2 + d2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

float boxesIntersection(const tk::dnn::box &a, const tk::dnn::box &b){
    float width  = boxesOverlap(a.x, a.w, b.x, b.w);
    float height = boxesOverlap(a.y, a.h, b.y, b.h);
    if(width < 0 || height < 0) 
        return 0;
    float area = width*height;
    return area;
}

float max_possible_intersection(const tk::dnn::box &a, const tk::dnn::box &b)
{
    float w = a.w < b.w ? a.w : b.w;
    float h = a.h < b.h ? a.h : b.h;
    return w*h;
}

// Intersection over max possible intersection
float IoMax(const tk::dnn::box &a, const tk::dnn::box &b)
{
    float I = boxesIntersection(a,b);
    float max = max_possible_intersection(a,b);
    return(I/max);
}

bool filter_detection_by_IoMax(const tk::dnn::box & detection, const tk::dnn::box &roi, const float roi_threshold)
{
    // Calculate max intersection between detection bbox and ROI bbox
    float io_max = IoMax(detection, roi);
    bool keep = (io_max >= roi_threshold);
    return(keep);
}

bool any_detections(const std::vector<tk::dnn::box> &detection_boxes, const std::vector<int> &trimmer_class_nums)
{
    // If the vector of bounding boxes is empty,
    // then there were no detections. Return false.
    if(detection_boxes.empty())
    {
        return(false);
    }

    // If the vector of bounding boxes is not empty, 
    // then figure out whether the detected class numbers
    // appear in the list of class numbers we would like
    // to include in the trimmed video.
    std::vector<int> detected_class_numbers(detection_boxes.size());
    for(int i = 0; i < detection_boxes.size(); i++){
        detected_class_numbers[i] = detection_boxes[i].cl;
    }

    // Uniquify the detect_class_numbers vector
    // to return only the unique class numbers
    std::sort(detected_class_numbers.begin(), detected_class_numbers.end());
    std::vector<int>::iterator newEnd;
    newEnd = std::unique(detected_class_numbers.begin(), detected_class_numbers.end());
    detected_class_numbers.erase(newEnd, detected_class_numbers.end());
    
    // Flag for "any detections?"
    bool any = false;

    // Loop over the detected class numbers and
    // and figure out whether any of them are included
    // in the list of classes on which we want to trim 
    for(int i = 0; i < detected_class_numbers.size(); i++)
    {
        int class_num = detected_class_numbers[i];
        for(int n = 0; n < trimmer_class_nums.size(); n++)
        {
            if(class_num == trimmer_class_nums[n])
            {
                any = true;
                break;
            }
        }
        if(any){break;}
    }
    return(any);
}

void get_valid_detections(std::vector<tk::dnn::box> &valid_detections, const std::vector<tk::dnn::box> &all_detections, const std::vector<int> &trimmer_class_nums)
{
    // If no ROI is specified then set ROI to whole frame
    tk::dnn::box roi;
    roi.x = 0;
    roi.y = 0;
    roi.w = 1;
    roi.h = 1;
    float roi_threshold = 0;
    get_valid_detections(valid_detections, all_detections, trimmer_class_nums, roi, roi_threshold);
}

void get_valid_detections(std::vector<tk::dnn::box> &valid_detections, const std::vector<tk::dnn::box> &all_detections, const std::vector<int> &trimmer_class_nums, const tk::dnn::box &roi, const float roi_threshold)
{

    // Keep only the detections with class numbers in trimmer_class_nums 
    std::vector<tk::dnn::box> in_class_detections;
    for(int i = 0; i < all_detections.size(); i++)
    {
        if(filter_detection_by_class(all_detections[i], trimmer_class_nums))
        {
            in_class_detections.push_back(all_detections[i]);
        }
    }

    // Keep only the detections that meet the IoU threshold criterion
    for(int i = 0; i < in_class_detections.size(); i++)
    {
        if(filter_detection_by_IoMax(in_class_detections[i], roi, roi_threshold))
        {
            valid_detections.push_back(in_class_detections[i]);
        }
    }
}

bool filter_detection_by_class(const tk::dnn::box &detection, const std::vector<int> &trimmer_class_nums)
{
    // Class number of this detection
    int detection_class_num = detection.cl;

    // Flag true if detection is in allowed classes, else false
    bool keep = false; 

    // Loop over all the allowed class numbers
    for(int n = 0; n < trimmer_class_nums.size(); n++)
        {
            if(detection_class_num == trimmer_class_nums[n])
            {
                keep = true;
                break;
            }
        }
    return(keep);
}