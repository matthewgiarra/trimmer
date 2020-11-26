DATA_DIR=$HOME/data
MODELS_DIR=$(pwd)/models
docker run --rm -it -v $(pwd):/workspace -v $DATA_DIR:/data -v $MODELS_DIR:/models --gpus all trimmer ./build/trimmer config/trimmer_config.json