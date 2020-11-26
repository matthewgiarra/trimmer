FROM ceccocats/tkdnn:latest
LABEL maintainer "Matthew N. Giarra <matthew.giarra@gmail.com>"

# Install boost libraries
RUN apt-get update && apt-get install libboost-filesystem-dev libboost-program-options1.65-dev -y
WORKDIR /workspace

# Compile tkDNN
WORKDIR /
RUN git clone https://github.com/ceccocats/tkDNN.git && cd tkDNN && mkdir build && cd build \ 
    && cmake .. && make -j12

# Download coco validation data
RUN /bin/bash /tkDNN/scripts/download_validation.sh COCO
WORKDIR /workspace
