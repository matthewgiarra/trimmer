#include "frame.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"
#include "BoundingBox.h"
#include "nlohmann/json.hpp"
#include "msgpack.hpp"
#include <zmq.hpp>

Frame::Frame()
{
    detection_boxes.clear();
}

Frame::Frame(zmq::message_t &recv_msg)
{
    detection_boxes.clear();
    populate(recv_msg);
}

void Frame::populate(zmq::message_t &recv_msg)
{
    if(recv_msg.size() > 0)
    {
        msgpack::unpacked result;
        result = msgpack::unpack(static_cast<const char*>(recv_msg.data()), recv_msg.size());
        msgpack::object deserialized = result.get();
        msgpack::type::tuple<int, std::string, std::vector<uint8_t>> dst;
        deserialized.convert(dst);
        
        // Get header
        header = nlohmann::json::parse(std::get<1>(dst));
        int rows = header["shape"][0];
        int cols = header["shape"][1];
        int channels = header["shape"][2];

        // Get image data
        std::vector<uint8_t> image_data = std::get<2>(dst);

        // This allocates the cv::Mat for holding the image data
        image.create(rows, cols, CV_8UC3);
        memcpy(image.data, image_data.data(), image_data.size()*sizeof(uint8_t));
    }
}