#!/bin/bash

if [ "$#" -eq 0 ]; then
    TARGET="trimmer"
else
    TARGET=$1
fi

docker run --rm -it -v $(pwd):/workspace --gpus all trimmer /bin/bash -c "mkdir -p /workspace/build && cd /workspace/build && cmake .. && make $TARGET -j12"