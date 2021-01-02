#!/bin/bash
DATA_DIR=$(pwd)/videos
docker run --rm -it -v $(pwd):/workspace -v $DATA_DIR:/data --net=host trimmer ./build/zmqtest 8888