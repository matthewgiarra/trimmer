#!/bin/bash

# Figure out where the scripts directory is (based on where this file is running)
SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Build source codes for this project
/bin/bash "$SCRIPTS_DIR/build_source.sh" 

# Build the models needed to run inference (required for this project)
/bin/bash "$SCRIPTS_DIR/build_models.sh"