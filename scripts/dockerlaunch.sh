DATA_DIR=$HOME/data

docker run --rm -it -v $(pwd):/workspace -v $DATA_DIR:/data --gpus all trimmer /bin/bash
