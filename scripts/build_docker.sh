#!/bin/bash

# Absolute path to the scripts directory
SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Checkout tkDNN submodule from my fork and build the image
# git submodule update --init --recursive
# cd tkDNN
# git checkout dockerfile.base-add-libyaml-cpp-dev
cd tkDNN/docker && docker build --rm -t ceccocats/tkdnn:latest -f Dockerfile.base .
# cd $SCRIPTS_DIR

# Build the docker image
docker build -t trimmer -f "$SCRIPTS_DIR/../Dockerfile" "$SCRIPTS_DIR/.."