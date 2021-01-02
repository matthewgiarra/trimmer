#!/bin/bash

if [ "$#" -eq 0 ]; then
    TARGET="trimmer"
else
    TARGET=$1
fi

if [ "$2" == "--nogpu" ]; then
    GPU_STRING=""
    echo "Building WITHOUT GPUs"
else
    GPU_STRING="--gpus all"
    echo "Building with GPUs"
fi

docker run --rm -it -v $(pwd):/workspace $GPU_STRING trimmer /bin/bash -c "mkdir -p /workspace/build && cd /workspace/build && cmake .. && make $TARGET -j12"