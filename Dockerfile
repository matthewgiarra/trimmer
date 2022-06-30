FROM ceccocats/tkdnn:latest
# FROM tkdnn:latest

LABEL maintainer "Matthew N. Giarra <matthew.giarra@gmail.com>"

# Update nvidia keys beacuse they broke the image or something
# RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/$(arch)/3bf863cc.pub

# Install boost libraries
RUN apt-get update && apt-get install libboost-filesystem-dev libboost-program-options-dev -y

# Where tkDNN is installed in the container
ARG TKDNN_ROOT="/root/Development/tkDNN"
ENV LD_LIBRARY_PATH="$TKDNN_ROOT/build:${LD_LIBRARY_PATH}"

# Download coco validation 
RUN cd $TKDNN_ROOT && /bin/bash $TKDNN_ROOT/scripts/download_validation.sh COCO
RUN mv $TKDNN_ROOT/demo/COCO_val2017 /COCO_val2017
WORKDIR /workspace