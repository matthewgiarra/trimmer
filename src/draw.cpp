#include "draw.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "BoundingBox.h"
#include "utils.h" // for getColor
#include <vector>
#include <string>

void get_box_colors(std::vector<cv::Scalar> &colors, int num_classes)
{
    // Clear the colors vector
    colors.clear();

    // class colors precompute    
    for(int c=0; c<num_classes; c++) {
        int offset = c*123457 % num_classes;
        float r = getColor(2, offset, num_classes);
        float g = getColor(1, offset, num_classes);
        float b = getColor(0, offset, num_classes);
        colors.push_back(cv::Scalar(int(255.0*b), int(255.0*g), int(255.0*r)));
    }
}

void draw_box_on_frame(cv::Mat &frame, tk::dnn::box &detection_box, const std::vector<std::string> &class_names)
{
    // Default to yellow rectangles
    draw_box_on_frame(frame, detection_box, cv::Scalar(255,255,0), class_names);
}

void draw_box_on_frame(cv::Mat &frame, tk::dnn::box &detection_box, const cv::Scalar &color, const std::vector<std::string> &class_names)
{
    int baseline = 0;
    float font_scale = 0.5;
    int thickness = 2;
    std::string det_class_name = class_names[detection_box.cl];

    int x0, x1, y0, y1;

    // Extents of bounding box
    x0 = detection_box.x;
    x1 = detection_box.x + detection_box.w;
    y0 = detection_box.y;
    y1 = detection_box.y + detection_box.h;

    // Draw the rectangle
    cv::rectangle(frame, cv::Point(x0, y0), cv::Point(x1, y1), color, thickness);

    // Draw the class name text
    cv::Size text_size = getTextSize(det_class_name, cv::FONT_HERSHEY_SIMPLEX, font_scale, thickness, &baseline);
    cv::rectangle(frame, cv::Point(x0, y0), cv::Point((x0 + text_size.width - 2), (y0 - text_size.height - 2)), color, -1);                      
    cv::putText(frame, det_class_name, cv::Point(x0, (y0 - (baseline / 2))), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), thickness);
}

void draw_boxes_on_frame(cv::Mat &frame, std::vector<tk::dnn::box> &detection_boxes, const std::vector<cv::Scalar> &colors, const std::vector<std::string> &class_names)
{
    for(int i = 0; i < detection_boxes.size(); i++)
    { 
        cv::Scalar color = colors[detection_boxes[i].cl];
        draw_box_on_frame(frame, detection_boxes[i], color, class_names);
    }
}

void draw_timestamp(cv::Mat &frame, long time_ms)
{
    int thickness = 2;
    long ms_per_hour = 3.6E6;
    long ms_per_min = 6E4;
    long ms_per_sec = 1E3;

    long hr = time_ms / ms_per_hour;
    time_ms -= hr * ms_per_hour;

    long min = time_ms / ms_per_min;
    time_ms -= min * ms_per_min;
    
    long sec = time_ms / 1000;
    time_ms -= sec * 1000;

    int pix_height = frame.rows / 20;
    double font_scale = cv::getFontScaleFromHeight(cv::FONT_HERSHEY_SIMPLEX, pix_height, thickness);
    int origin_y = frame.rows / 20; 

    std::string time_stamp = std::to_string(hr) + ":" + std::to_string(min) + ":" + std::to_string(sec) + "." + std::to_string(time_ms);

    cv::Size text_size = getTextSize(time_stamp, cv::FONT_HERSHEY_SIMPLEX, font_scale, thickness, 0);
    cv::rectangle(frame, cv::Point(0, origin_y), cv::Point((text_size.width - 2), (origin_y - text_size.height - 2)), cv::Scalar(255, 255, 255), -1);                      
    cv::putText(frame, time_stamp, cv::Point(0, origin_y), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0),thickness);
}
