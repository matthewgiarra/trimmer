#!/bin/bash
# This script generates the following YOLOv4 models needed to perform inference in tkDNN:
# yolo4_fp32.rt
# yolo4_fp16.rt
# yolo4_int8.rt
# yolo4tiny_fp32.rt
# yolo4tiny_fp16.rt
# yolo4tiny_int8.rt

# The different models perform differently:
# speed:    yolo4 < yolo4tiny, fp32 < fp16 < int8
# accuracy: yolo4 > yoloytiny, fp32 > fp16 > int8

# To regenerate models, delete the existing model file
# at models/<model_name.rt> and re-run this script

# The maximum batch size allowed by the models produced is specified by TKDNN_BATCHSIZE.
# Modify the TKDNN_BATCHSIZE variable below to vary the max batch size from 1-64.
# TKDNN_BATCHSIZE=16 seems to work well on an RTX8000 (~400 FPS for yolo4_int8.rt)
# YMMV so experiment with this
TKDNN_BATCHSIZE=16
TKDNN_ROOT="/root/Development/tkDNN"

# Calibration images root dir
CALIB_ROOT="$TKDNN_ROOT/demo/COCO_val2017"

# Check if this is being run as root
if [ -f /.dockerenv ] # This line checks whether the script is running in a docker container
then
    echo "Building yolo4 and yolo4tiny detector models for inference ..."
    
    # Paths to coco labels and images for int8 calibration 
    export TKDNN_CALIB_LABEL_PATH="$CALIB_ROOT/all_labels.txt"
    export TKDNN_CALIB_IMG_PATH="$CALIB_ROOT/all_images.txt"    

    # cd /models
    cd "$TKDNN_ROOT/build"

    # Build yolo4 fp32
    FILE="yolo4_fp32.rt"
    if [ ! -f "/models/$FILE" ]
    then
        echo "Building $FILE"
        export TKDNN_MODE=FP32
        /bin/bash -c "$TKDNN_ROOT/build/test_yolo4"
        test -f $FILE && echo "Built $FILE"
        cp $FILE "/models/$FILE"
    else
        echo "/models/$FILE exists, skipping"
    fi
    
    # Build yolo4 fp16
    FILE="yolo4_fp16.rt"
    if [ ! -f "/models/$FILE" ]
    then
        echo "Building $FILE"
        export TKDNN_MODE=FP16
        /bin/bash -c "$TKDNN_ROOT/build/test_yolo4"
        test -f $FILE && echo "Built $FILE"
        cp $FILE "/models/$FILE"
    else
        echo "/models/$FILE exists, skipping"
    fi

    # Build yolo4 int8
    FILE="yolo4_int8.rt"
    if [ ! -f "/models/$FILE" ]
    then
        echo "Building $FILE"
        export TKDNN_MODE=INT8
        /bin/bash -c "$TKDNN_ROOT/build/test_yolo4"
        test -f $FILE && echo "Built $FILE"
        cp $FILE "/models/$FILE"
    else
        echo "/models/$FILE exists, skipping"
    fi

    # Build yolo4-tiny fp32
    FILE="yolo4tiny_fp32.rt"
    if [ ! -f "/models/$FILE" ]
    then
        echo "Building $FILE"
        export TKDNN_MODE=FP32
        /bin/bash -c "$TKDNN_ROOT/build/test_yolo4tiny"
        test -f $FILE && echo "Built $FILE"
        cp $FILE "/models/$FILE"
    else
        echo "/models/$FILE exists, skipping"
    fi

    # Build yolo4-tiny fp16
    FILE="yolo4tiny_fp16.rt"
    if [ ! -f "/models/$FILE" ]
    then
        echo "Building yolo4tiny (fp16)"
        export TKDNN_MODE=FP16
        /bin/bash -c "$TKDNN_ROOT/build/test_yolo4tiny"
        test -f $FILE && echo "Built $FILE"
        cp $FILE "/models/$FILE"
    else
        echo "/models/$FILE exists, skipping"
    fi

    # Build yolo4tiny int8
    FILE="yolo4tiny_int8.rt"
    if [ ! -f "/models/$FILE" ]
    then
        echo "Building yolo4 tiny (int8)"
        export TKDNN_MODE=INT8
        /bin/bash -c "$TKDNN_ROOT/build/test_yolo4tiny"
        test -f $FILE && echo "Built $FILE"
        cp $FILE "/models/$FILE"
    else
        echo "/models/$FILE exists, skipping"
    fi

else
    # This line returns the path to the directory containing this script (the one you are reading)
    SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

    # Create the model directory
    MODEL_DIR_HOST="$SCRIPTS_DIR/../models"
    mkdir -p $MODEL_DIR_HOST
    test ! -d $MODEL_DIR_HOST && raise error "Failed to create $MODEL_DIR_HOST" && exit 1
    
    # Launch the docker container to build the models
    docker run --rm -it -v $(pwd):/workspace -v $MODEL_DIR_HOST:/models --env TKDNN_BATCHSIZE=$TKDNN_BATCHSIZE --env TKDNN_ROOT=$TKDNN_ROOT --gpus all trimmer /bin/bash -c "/workspace/scripts/build_models.sh"
fi
