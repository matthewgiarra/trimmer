#include "bbox.hpp"
#include "BoundingBox.h"
#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>

BBox::BBox()
{

}

BBox::BBox(tk::dnn::box &box_in, std::string &class_name)
{

}

BBox::BBox(tk::dnn::box &box_in, std::vector<std::string> &class_names)
{

}

BBox::BBox(int x_input, int y_input, int w_input, int h_input, int class_id_input, std::string &class_name_input)
{
    x = x_input;
    y = y_input;
    w = w_input;
    h = h_input;
    class_id = class_id_input;
    class_name = class_name_input;
}

BBox::BBox(int x_input, int y_input, int w_input, int h_input, int class_id_input, std::vector<std::string> &class_names)
{
    x = x_input;
    y = y_input;
    w = w_input;
    h = h_input;
    class_id = class_id_input;
    class_name = class_names[class_id];
}

void draw_box_on_frame(cv::Mat &frame, BBox &bounding_box)
{

    // Default to yellow rectangles
    draw_box_on_frame(frame, bounding_box, cv::Scalar(255,255,0));
}

void draw_box_on_frame(cv::Mat &frame, BBox &bounding_box, const cv::Scalar &color)
{
    int baseline = 0;
    float font_scale = 0.5;
    int thickness = 2;
    std::string det_class_name = bounding_box.class_name;

    int x0, x1, y0, y1;

    // Extents of bounding box
    x0 = bounding_box.x;
    x1 = bounding_box.x + bounding_box.w;
    y0 = bounding_box.y;
    y1 = bounding_box.y + bounding_box.h;

    // Draw the rectangle
    cv::rectangle(frame, cv::Point(x0, y0), cv::Point(x1, y1), color, thickness);

    // Draw the class name text
    cv::Size text_size = getTextSize(det_class_name, cv::FONT_HERSHEY_SIMPLEX, font_scale, thickness, &baseline);
    cv::rectangle(frame, cv::Point(x0, y0), cv::Point((x0 + text_size.width - 2), (y0 - text_size.height - 2)), color, -1);                      
    cv::putText(frame, det_class_name, cv::Point(x0, (y0 - (baseline / 2))), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(255, 255, 255), thickness);
}

void draw_boxes_on_frame(cv::Mat &frame, std::vector<BBox> &bounding_boxes, std::vector<cv::Scalar> &colors)
{
    for(int i = 0; i < bounding_boxes.size(); i++)
    { 
        cv::Scalar color = colors[bounding_boxes[i].class_id];
        draw_box_on_frame(frame, bounding_boxes[i], color);
    }
}
