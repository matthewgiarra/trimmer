
// #include "BoundingBox.h"

#include "opencv2/core/mat.hpp"
#include "opencv2/core/core.hpp"

#include "CenternetDetection.h"
#include "MobilenetDetection.h"
#include "Yolo3Detection.h"
#include "BoundingBox.h"

#include "constants.hpp"
#include "nlohmann/json.hpp"
#include "boost/filesystem.hpp"
#include <cmath>
#include <iostream>
#include <mutex>
#include <fstream>
#include <string>
#include <queue>
#include <memory>
#include <thread>
#include <atomic>
#include "video.hpp"


std::atomic<bool> g_run_video_writer_thread(false);

int main(int argc, char *argv[])
{

    std::cout << "HELLO DINGUS" << std::endl;
    std::cout<<"TRIMMER: Object-detection-based video trimmer\n";

    g_run_video_writer_thread = true;

    // Path to the config file
    std::string config_filepath = std::string(argv[1]);

    int thread_num;
    int threads;

    // Thread number
    if(argc == 1 || argc == 3)
    {
        std::cerr << "Usage:" << std::endl << "./trimmer <path_to_config.json>"  << std::endl;
        std::cerr << "./trimmer <path_to_config.json> <thread_number> <number_of_threads>" << std::endl;
        return -ENOENT;
    }

    if(argc == 2)
    {
        thread_num = 0;
        threads = 1;
    }

    if(argc == 4)
    {
        thread_num = atoi(argv[2]);
        threads    = atoi(argv[3]);
    }


    // Some hard coded stuff for now
    char ntype = 'y';
    int n_classes = 80;

    // Shared pointer to video buffer for swapping b/t threads
    std::shared_ptr<std::queue<Video>> video_queue_buf_sp  = std::shared_ptr<std::queue<Video>>(new std::queue<Video>());
    std::shared_ptr<std::timed_mutex> video_queue_mutex_sp = std::shared_ptr<std::timed_mutex>(new std::timed_mutex());
    std::queue<Video> video_queue_detector;

    // Declare some detectors instances 
    tk::dnn::Yolo3Detection yolo;
    tk::dnn::CenternetDetection cnet;
    tk::dnn::MobilenetDetection mbnet;  
    tk::dnn::DetectionNN *detNN;

    switch(ntype)
    {
        case 'y':
            detNN = &yolo;
            break;
        case 'c':
            detNN = &cnet;
            break;
        case 'm':
            detNN = &mbnet;
            n_classes++;
            break;
        default:
        FatalError("Network type not allowed (3rd parameter)\n");
    }

    // Print the path to the json file that will be parsed.
    std::cout << "Trimmer config filepath: " << config_filepath << std::endl;
    
    // Open and parse the json config file
    std::ifstream input_stream;
    input_stream.open(config_filepath);
    if (!input_stream.is_open())
    {
        std::cerr << "Could not open input file invalid path.\tfilepath: " << config_filepath << std::endl;
        return -ENOENT;
    }
    nlohmann::json config_data = nlohmann::json::parse(input_stream);
    input_stream.close();

    // Path to the top-level data directory
    boost::filesystem::path data_dir_host             = config_data[g_files][g_data_dir_host];
    boost::filesystem::path data_dir_container        = config_data[g_files][g_data_dir_container];
    boost::filesystem::path workspace_dir_container   = config_data[g_files][g_workspace_dir_container];
    boost::filesystem::path video_list_path_workspace = config_data[g_files][g_video_list_path_workspace];
    boost::filesystem::path video_list_path_container = workspace_dir_container / video_list_path_workspace;

    if(!boost::filesystem::exists(video_list_path_container))
    {
        std::cerr << "Video list file not found: " << video_list_path_container.string() << std::endl;
        return -ENOENT;
    }
    else
    {
        std::cout << "Found video list file: " << video_list_path_container.string() << std::endl;
    }
    
    // Read the video list
    input_stream.open(video_list_path_container.string());
    if (!input_stream.is_open())
    {
        std::cerr << "Could not open input file invalid path.\tfilepath: " << video_list_path_container << std::endl;
        return -ENOENT;
    }
    nlohmann::json file_list_data = nlohmann::json::parse(input_stream);
    input_stream.close();

    // Turn the video list into a vector of boost filepaths
    std::vector<std::string> video_path_list_str = file_list_data[g_files];
    
    for(int i = 0; i < video_path_list_str.size(); i++)
    {
        // Absolute path to the file on the host
        boost::filesystem::path video_path_host_abs(video_path_list_str[i]);
        
        // Video path relative to the data directory on the host
        boost::filesystem::path video_path_host_rel = boost::filesystem::relative(video_path_host_abs, data_dir_host);

        // Absolute path to the video in the container
        // (made by joining relative host path to container data root path)
        boost::filesystem::path video_path_container = data_dir_container / video_path_host_rel;

        // Search for the file
        if(!boost::filesystem::exists(video_path_container))
        {
            std::cerr << "Video file not found:" <<std::endl;
            std::cerr << "Container path: " << video_path_container.string() << std::endl;
            std::cerr << "Host path: "      << video_path_host_abs.string() << std::endl;
            return -ENOENT;
        }
        else
        {
            // Add the video path (container path) to the vector of video paths  
            std::cout << "Adding video to list: " << video_path_host_abs.string() << std::endl;
            video_queue_detector.push(Video(video_path_container.string()));
        }
    }
    int num_videos = video_queue_detector.size();
    threads = std::min(threads, num_videos);
    if(thread_num > (threads - 1))
    {
        std::cerr << "No more videos to process! Exiting." << std::endl;
        return -ENOENT;
    }

    // Chunk the video queue into a vector of queues
    std::vector<std::queue<Video>> video_queue_list(threads);
 
    // Number of videos per chunk (last chunk might have fewer videos)
    const int chunk_size = (int) std::ceil((float)num_videos / (float)threads);

    // Populate all the chunks
    for(int c = 0; c < threads; c++)
    {
        for(int s = 0; s < chunk_size; s++)
        {
            if(!video_queue_detector.empty())
            {
                video_queue_list[c].push(video_queue_detector.front());
                video_queue_detector.pop();
            }
        }
    }

    // Now grab just the video queue for this thread
    if(!video_queue_detector.empty())
    {
        std::cerr << "Error: not all videos assigned to a thread" << std::endl;
        return -ENOENT;
    }
    while(!video_queue_list[thread_num].empty())
    {
        video_queue_detector.push(video_queue_list[thread_num].front());
        video_queue_list[thread_num].pop();
    }

    std::cout << "Preparing to process " << video_queue_detector.size() << " videos" << std::endl;

    // Initialize the network
    std::string net = config_data[g_files][g_model_path_container];
    float conf_thresh = config_data[g_parameters][g_confidence_threshold];
    int batch_size = config_data[g_parameters][g_batch_size];
    int frame_step = config_data[g_parameters][g_frame_step];
    if(batch_size < 1 || batch_size > 64)
        FatalError("Batch dim not supported");
    detNN->init(net, n_classes, batch_size, conf_thresh);

    // Print the names of all the classes in the model
    std::cout << "Classes in model: " << std::endl;
    for(int i = 0; i < n_classes; i++)
    {
        std::cout << detNN->classesNames[i] << std::endl;
    }
    
    // Get the class numbers of the class names specified in the config file
    std::vector<std::string> trimmer_class_names = config_data[g_classes];
    std::vector<int> trimmer_class_nums(trimmer_class_names.size());
    for(int i = 0; i < trimmer_class_names.size(); i++){
        for(int j = 0; j < n_classes; j++){
            if(detNN->classesNames[j] == trimmer_class_names[i])
            {
                trimmer_class_nums[i] = j;
                break;
            }
        }
    }
    
    // Print the classes we're trimming on
    std::cout << "Trimming on classes:" << std::endl;
    for(int i = 0; i < trimmer_class_names.size(); i++){
        std::cout << trimmer_class_names[i] << " (" << trimmer_class_nums[i] << ")" << std::endl;
    }
    // Start the video writer thread 
    std::thread video_writer_thread(run_video_writer_thread, video_queue_mutex_sp, video_queue_buf_sp, config_filepath, trimmer_class_nums, detNN->classesNames);
    
    // Loop over the videos
    while(!video_queue_detector.empty())
    {
        // Image height and width
        int image_height, image_width;
        
        // Video path as a string
        std::string video_path = video_queue_detector.front().path;
        bool gRun = true;
        cv::VideoCapture cap(video_path);
        if(!cap.isOpened())
        {
            std::cerr << "Failed to open video file: " << video_path << std::endl;
            gRun = false;
        }
        else
        {
            std::cout<<"Opened " << video_path << " in detector" << std::endl;
            image_width  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
            image_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        }
            
        // Declare frame holding image data 
        cv::Mat frame;            
        std::vector<cv::Mat> batch_frame;
        std::vector<cv::Mat> batch_dnn_input;
        tk::dnn::box tk_bbox;
        std::string class_name;
        
        // Number of batches processed
        int batch_num = 0;
        while(gRun)
        {
            batch_dnn_input.clear();
            batch_frame.clear();
            for(int bi=0; bi< batch_size; ++bi)
            {
                // Read the frame
                for(int f = 0; f < frame_step; f++)
                {
                    cap.read(frame);
                }
                if(!frame.data)
                {
                    gRun = false;
                    break;
                }
                    
                // Frame for drawing I think
                batch_frame.push_back(frame);

                // this will be resized to the net format
                batch_dnn_input.push_back(frame.clone());
            }
            
            if(batch_dnn_input.empty())
            {
                // No frames in batch; done with this video
                break;
            }

            // A few frames in this batch; need to pad with zeros
            // so the inference doesn't barf on an unexpected batch size
            if(batch_dnn_input.size() < batch_size)
            {
                int batch_deficit = batch_size - batch_dnn_input.size();
                for(int i = 0; i < batch_deficit; i++)
                {
                    batch_dnn_input.push_back(cv::Mat::zeros(image_height, image_width, CV_32F));
                }
            }
                
            // Do the inference
            detNN->update(batch_dnn_input, batch_size);
            detNN->draw(batch_frame);

            // Determine if any of the specified classes were detected
            // Loop over the frames in the batch
            for(int bi=0; bi<batch_frame.size(); ++bi)
            {
                // Frame number
                int frame_num = batch_num * batch_size + bi * frame_step;
                video_queue_detector.front().detection_framenums.push(frame_num);

                // Detections
                video_queue_detector.front().detection_boxes.push(detNN->batchDetected[bi]);
            }
            // Increment the number of processed batches
            batch_num++;     
        }

        // Push the finished video to the shared video queue
        std::unique_lock<std::timed_mutex> data_lock(*video_queue_mutex_sp, std::defer_lock);
        data_lock.lock();
        video_queue_buf_sp->push(video_queue_detector.front());
        data_lock.unlock();
        video_queue_detector.pop();
        
        std::cout << "Finished detections in " << video_path << std::endl;
    }

    g_run_video_writer_thread = false;
    video_writer_thread.join();
    std::cout<<"Done with batch\n";   
    double mean = 0; 
    
    std::cout<<COL_GREENB<<"\n\nTime stats:\n";
    std::cout<<"Min: "<<*std::min_element(detNN->stats.begin(), detNN->stats.end())/batch_size<<" ms\n";    
    std::cout<<"Max: "<<*std::max_element(detNN->stats.begin(), detNN->stats.end())/batch_size<<" ms\n";    
    for(int i=0; i<detNN->stats.size(); i++) mean += detNN->stats[i]; mean /= detNN->stats.size();
    std::cout<<"Avg: "<<mean/batch_size<<" ms\t"<<1000/(mean/batch_size)<<" FPS\n"<<COL_END; 
    
    return 0;
}

