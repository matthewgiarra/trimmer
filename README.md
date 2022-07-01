

# trimmer
Trim long videos to just the segments containing objects of interest. 

# Quick start

1. Checkout the working version.
*Note: This is temporary until the parent repository (ceccocats/tkNN) fixes their dockerfile.*
```bash
git clone --recurse-submodules git@github.com:matthewgiarra/trimmer.git
cd trimmer
git checkout modernize
git submodule update --init --recursive
cd tkDNN
git checkout dockerfile.base-add-libyaml-cpp-dev
cd ..
```
2. Build everything
```Bash
# Build docker images
./scripts/build_docker.sh

# Build trimmer application
./scripts/build_source.sh

# Build models required to run trimmer application
./scripts/build_models.sh

# Run trimmer on the example video
./scripts/main.sh
```

# Step-by-step build
Follow these instructions if the scripts-only build above gives you trouble. 
1. Clone this repository including submodules

```bash
git clone --recurse-submodules https://github.com/matthewgiarra/trimmer
```

2. Switch to the `modernize` branch
```bash
cd trimmer
git checkout modernize
```

3. Checkout the fork of `tkDNN`. *Note: we're only using this custom branch until the parent repository fixes its dockerfile, which is currently broken*
```bash
git submodule update --init
cd tkDNN
git checkout dockerfile.base-add-libyaml-cpp-dev
```
4. Build the `tkDNN` base image. This will take a long time.
```bash
cd docker
docker build --rm -t ceccocats/tkdnn:latest -f Dockerfile.base . 
```

5. Build the `trimmer` image
```bash
cd ../.. # Go back up to the trimmer base directory

# Do this from the top-level trimmer directory
docker build --rm -t trimmer .
```

6. Compile the `trimmer` application
```bash
# Do this from the top-level trimmer directory
./scripts/build_source.sh
```

7. Build the models required to run the `trimmer` application. This will take a long time. 
```bash
# Do this from the top-level trimmer directory
./scripts/build_models.sh
```
8. Run the `trimmer` application on the example video

