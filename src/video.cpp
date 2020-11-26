#include "video.hpp"
#include "boost/filesystem.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"
#include "opencv2/videoio.hpp"
#include "nlohmann/json.hpp"
#include "constants.hpp"
#include <atomic>
#include <queue>
#include <iostream>
#include <mutex>
#include <memory>
#include <thread>

Video::Video(std::string &input_file_path)
{
    finished = false;
    path = input_file_path;
    
};

Video::Video(boost::filesystem::path &input_file_path)
{
    finished = false;
    path = input_file_path.string();
    
};


int write_result_video(Video &video, const std::string &config_filepath)
{
    // Read the config file
    std::ifstream input_stream;
    input_stream.open(config_filepath);
    if (!input_stream.is_open())
    {
        std::cerr << "Could not open input file invalid path.\tfilepath: " << config_filepath << std::endl;
        return -ENOENT;
    }
    nlohmann::json config_data = nlohmann::json::parse(input_stream);
    input_stream.close();

    // Get parameters
    boost::filesystem::path input_path(video.path);
    boost::filesystem::path output_file_name(input_path.stem().string() + config_data[g_files][g_output_video_suffix].get<std::string>() + input_path.extension().string());
    
    // Choose the output directory
    // If consolidating video outputs, put everything in output_dir_container
    // Otherwise put the output video next to the input video with suffix appended before extension 
    boost::filesystem::path output_dir_container;
    if(config_data[g_options][g_consolidate_videos]){
        output_dir_container = config_data[g_files][g_output_dir_container].get<std::string>();
    }
    else
    {
        output_dir_container = input_path.parent_path();
    }

    // Create the output directory
    boost::filesystem::create_directories(output_dir_container);

    // Construct the output file path (container path)
    boost::filesystem::path output_filepath_container = output_dir_container / output_file_name;

    // Open the file for reading
    cv::VideoCapture cap(video.path);
    if(!cap.isOpened())
    {
        std::cerr << "Failed to open video file: " << video.path << std::endl;
        return(-1); //TODO: More descriptive return for error tracking
    }
    
    // Get some parameters from the input video
    int image_width  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int image_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fourcc = cap.get(cv::CAP_PROP_FOURCC);
    double fps = cap.get(cv::CAP_PROP_FPS);

    cv::VideoWriter resultVideo;
    resultVideo.open(output_filepath_container.string(), fourcc, fps, cv::Size(image_width, image_height));

    // Frame
    cv::Mat frame;

    int frame_num = 0;
    int frames_written = 0;
    while(!video.detection_framenums.empty())
    {
        cap >> frame;
        if(!frame.data)
        {
            break;
        }
        if(frame_num == video.detection_framenums.front())
        {
            // Add the frame to the video
            resultVideo << frame;

            // Pop the element from the queue
            video.detection_framenums.pop();

            // Increment the number of frames written
            frames_written++;
        }
        // Increment frame number
        frame_num += 1;
    }
    std::cout << "Wrote " << frames_written << " frames to " << output_filepath_container.string() << std::endl;

    //TODO: Check that video actually got written

    return(0);
}

extern std::atomic<bool> g_run_video_writer_thread;
void run_video_writer_thread(std::shared_ptr<std::timed_mutex> video_queue_mutex_sp, std::shared_ptr<std::queue<Video>> video_queue_buf_sp, const std::string &config_filepath)
{    
    // Make a queue for writing videos
    std::queue<Video> video_queue_writer;
    while(g_run_video_writer_thread)
    {
        std::unique_lock<std::timed_mutex> data_lock(*video_queue_mutex_sp, std::defer_lock);
        if(data_lock.try_lock_for(std::chrono::milliseconds(10)))
        {
            
            // Load up the writer queue with all the 
            // videos from the buffer queue
            // (detections are done on these vids)
            while(!video_queue_buf_sp->empty())
            {
                video_queue_writer.push(video_queue_buf_sp->front());
                video_queue_buf_sp->pop();
            }
        }

        // Export all the videos in the queue
        while(!video_queue_writer.empty())
        {            
            write_result_video(video_queue_writer.front(), config_filepath);
            video_queue_writer.pop();
        }        
    }
}