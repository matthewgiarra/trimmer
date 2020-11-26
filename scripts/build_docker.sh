#!/bin/bash

# Absolute path to the scripts directory
SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Build the docker image
docker build --rm -t trimmer -f "$SCRIPTS_DIR/../Dockerfile" "$SCRIPTS_DIR/.."