
#include "video.hpp"
#include "draw.hpp"
#include "constants.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"
#include "opencv2/videoio.hpp"
#include "BoundingBox.h"
#include "boost/filesystem.hpp"
#include "nlohmann/json.hpp"

#include <atomic>
#include <queue>
#include <iostream>
#include <mutex>
#include <memory>
#include <thread>
#include <string>
#include <algorithm>

Video::Video(const std::string &input_file_path)
{
    finished = false;
    path = input_file_path;
    
};

// Video::Video(boost::filesystem::path &input_file_path)
// {
//     finished = false;
//     path = input_file_path.string();
    
// };


/*
    // Initialize 'detected' value as false
    bool trimmer_classes_detected = false;

    // 

    // Loop over the detections for the bi'th frame in the batch
    for(int i=0; i < detNN->batchDetected[bi].size(); i++) { 

        // Get info for the ith detection in the bi'th batch index
        tk_bbox = detNN->batchDetected[bi][i];
        
        // Class number of detection
        for(int j = 0; j < trimmer_class_nums.size(); j++)
        {
            if(trimmer_class_nums[j] == tk_bbox.cl)
            {
                // Draw box
            }
        }

        if(trimmer_classes_detected)
        { 
            break;
        }
    }
    */

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

void get_valid_detections(std::vector<tk::dnn::box> &valid_detections, const std::vector<tk::dnn::box> &all_detections, const std::vector<int> trimmer_class_nums)
{
    int num_detections = all_detections.size();
    int num_trimmer_classes = trimmer_class_nums.size();
    
    for(int i = 0; i < num_detections; i++)
    {
        int det_class_num = all_detections[i].cl;
        for(int n = 0; n < num_trimmer_classes; n++)
        {
            if(det_class_num == trimmer_class_nums[n])
            {
                valid_detections.push_back(all_detections[i]);
                break;
            }
        }
    }
}

int write_result_video(Video &video, const std::string &config_filepath, const std::vector<int> &trimmer_class_nums, const std::vector<std::string> &class_names)
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

    // Processing options
    bool trim_videos = config_data[g_options][g_trim_videos];
    bool draw_boxes  = config_data[g_options][g_draw_boxes];
    int frame_step   = config_data[g_parameters][g_frame_step];

    // Get bounding box colors
    std::vector<cv::Scalar> box_colors;
    get_box_colors(box_colors, class_names.size());

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

    cv::VideoWriter result_video;
    result_video.open(output_filepath_container.string(), fourcc, fps, cv::Size(image_width, image_height));

    // Frame
    cv::Mat frame;

    int frame_num = 0;
    int frames_written = 0;
    while(!video.detection_framenums.empty())
    {
        for(int f = 0; f < frame_step; f++)
        {
            cap.read(frame);
        }
        if(!frame.data)
        {
            break;
        }

        // Write this frame?
        bool write_frame = (!trim_videos) || any_detections(video.detection_boxes.front(), trimmer_class_nums);
        if(write_frame)
        {
            // TODO: add (if(draw_boxes) {//draw_boxes})
            if(draw_boxes)
            {
                std::vector<tk::dnn::box> valid_detections;
                get_valid_detections(valid_detections, video.detection_boxes.front(), trimmer_class_nums);
                draw_boxes_on_frame(frame, valid_detections, box_colors, class_names);
            }
            result_video << frame;
            frames_written++;
        }
        video.detection_framenums.pop();
        video.detection_boxes.pop();
        
        // Increment frame number
        frame_num += 1;
    }
    std::cout << "Wrote " << frames_written << " frames to " << output_filepath_container.string() << std::endl;

    //TODO: Check that video actually got written

    return(0);
}

extern std::atomic<bool> g_run_video_writer_thread;
void run_video_writer_thread(std::shared_ptr<std::timed_mutex> video_queue_mutex_sp, std::shared_ptr<std::queue<Video>> video_queue_buf_sp, const std::string &config_filepath, const std::vector<int> &trimmer_class_nums, const std::vector<std::string> &class_names)
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
            write_result_video(video_queue_writer.front(), config_filepath, trimmer_class_nums, class_names);
            video_queue_writer.pop();
        }        
    }
}