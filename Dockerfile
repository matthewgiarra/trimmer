FROM ceccocats/tkdnn:latest
LABEL maintainer "Matthew N. Giarra <matthew.giarra@gmail.com>"

# Update nvidia keys beacuse they broke the image or something
RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/$(arch)/3bf863cc.pub

# Install boost libraries
RUN apt-get update && apt-get install libboost-filesystem-dev libboost-program-options-dev -y

# Download coco validation 
RUN mv /root/Development/tkDNN /tkDNN
RUN cd /tkDNN && /bin/bash scripts/download_validation.sh COCO
WORKDIR /workspace
