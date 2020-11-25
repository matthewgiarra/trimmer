FROM ceccocats/tkdnn:latest
LABEL maintainer "Matthew Giarra"

# Install boost libraries
RUN apt-get update && apt-get install libboost-filesystem-dev libboost-program-options1.65-dev -y
WORKDIR /workspace

# Compile tkDNN
WORKDIR /
RUN git clone https://github.com/ceccocats/tkDNN.git && cd tkDNN && mkdir build && cd build \ 
    && cmake .. && make -j12

# Put compiling models here 

WORKDIR /workspace
