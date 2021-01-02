#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include "nlohmann/json.hpp"
#include "msgpack.hpp"
#include "opencv2/core/mat.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "frame.hpp"

static void parse_message_header(zmq::message_t &recv_msg, nlohmann::json &header_json)
{
  if (recv_msg.size() > 0)
  {    
      void *msg_ptr = recv_msg.data();
      int *header_bytes_ptr = (int*)msg_ptr;
      int  header_bytes = *header_bytes_ptr;
      char *header_ptr = (char*)&header_bytes_ptr[1]; // Points to the first byte in the header text
      std::string header_str(header_ptr, header_ptr + header_bytes);
      header_json = nlohmann::json::parse(header_str);
  }
};

void print_message_header(zmq::message_t &recv_msg)
{
  if (recv_msg.size() > 0)
  {    
      nlohmann::json header_json;
      parse_message_header(recv_msg, header_json);
      std::cout << header_json <<std::endl;
  }
};

int main (int argc, char *argv[])
{

    // Create zmq context
    zmq::context_t context;
    msgpack::unpacked result;
    
    // Port number
    int port;
    if(argc < 2)
    {
        port = 5555;
    }
    else
    {
        port = atoi(argv[1]);
    }
    std::cout << "Subscribing to tcp://localhost:" << port << std::endl;

    // Socket string
    std::string socket_str = std::string("tcp://localhost:") + std::to_string(port);

    //  Socket to talk to server
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect(socket_str);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    int num_gotten = 0;
    std::cout << "Waiting for message..." << std::endl;
    cv::VideoWriter result_video;
    int fourcc = cv::VideoWriter::fourcc('X', '2', '6', '4');
    int fps = 30;

    // Output file
    std::string output_filepath_container("/data/out.mp4");
    
    // Arbitrary number of frames for testing
    while(num_gotten < 300)
    {
        zmq::message_t recv_msg;
        subscriber.recv(&recv_msg);
        std::cout << "recv_msg.size() = " << recv_msg.size() << std::endl;
        if(recv_msg.size() > 0)
        {
            // Create frame object
            Frame frame(recv_msg);
            
            if(!result_video.isOpened())
            {

                int rows = frame.header["shape"][0];
                int cols = frame.header["shape"][1]; 

                // Open results video for writing
                result_video.open(output_filepath_container, fourcc, fps, cv::Size(cols, rows));
                
                // Verify the video got opened for writing
                if(!result_video.isOpened())
                {
                    std::cerr << "Error: failed to open file for writing: " << output_filepath_container << std::endl;
                    return -ENOENT;
                }
            }

            // Write frame to output video
            result_video.write(frame.image);
            
            // std::string header_str(header_ptr, header_ptr + header_bytes);
            std::cout << "Received message " << num_gotten << std::endl;
            num_gotten+=1;
        }
    }

    result_video.release();

    // GTFO
    return(0);
}