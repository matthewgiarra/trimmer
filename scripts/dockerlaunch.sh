DATA_DIR=$(pwd)/videos
MODELS_DIR=$(pwd)/models
docker run --rm -it -v $(pwd):/workspace -v $DATA_DIR:/data -v $MODELS_DIR:/models --gpus all trimmer /bin/bash
