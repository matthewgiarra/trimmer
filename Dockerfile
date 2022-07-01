FROM ceccocats/tkdnn:latest
LABEL maintainer "Matthew N. Giarra <matthew.giarra@gmail.com>"

# Install boost libraries
RUN apt-get update && apt-get install libboost-filesystem-dev libboost-program-options-dev -y

# Where tkDNN is installed in the image
ARG TKDNN_ROOT="/root/Development/tkDNN"
ENV TKDNN_ROOT=$TKDNN_ROOT

# Copy .so files to a dir on the system path
RUN cp $TKDNN_ROOT/build/*.so /usr/local/lib

# Download coco validation 
RUN cd $TKDNN_ROOT && /bin/bash $TKDNN_ROOT/scripts/download_validation.sh COCO

WORKDIR /workspace