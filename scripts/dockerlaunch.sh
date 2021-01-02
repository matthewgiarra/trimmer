DATA_DIR=$(pwd)/videos
MODELS_DIR=$(pwd)/models

if [ "$1" == "--nogpu" ]; then
    GPU_STRING=""
    echo "Launching WITHOUT GPUs"
else
    GPU_STRING="--gpus all"
    echo "Launching with GPUs"
fi

docker run --rm -it -v $(pwd):/workspace -v $DATA_DIR:/data -v $MODELS_DIR:/models $GPU_STRING trimmer /bin/bash
