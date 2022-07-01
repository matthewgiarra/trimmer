

# trimmer
Trim long videos to just the segments containing objects of interest. 

# Quick start

1. Clone this repo & submodules
*Note: This is temporary until the parent repository (ceccocats/tkNN) fixes their dockerfile.*
```bash
git clone --recurse-submodules git@github.com:matthewgiarra/trimmer.git # see Troubleshooting section if this fails.
cd trimmer/tkDNN
git checkout dockerfile.base-add-libyaml-cpp-dev # Need to check out this branch until parent repo fixes dockerfile
cd .. # This should put you in trimmer's main directory
```
2. Build everything
```Bash
# Build docker images. This takes a long time. 
./scripts/build_docker.sh

# Build trimmer application
./scripts/build_source.sh

# Build models required to run trimmer application
./scripts/build_models.sh

```

Or, alternatively, in one line:

```bash
./scripts/build_all.sh
```

3. Run the `trimmer` application on the example video.

```bash
# Run trimmer on the example video
./scripts/main.sh
```

# Troubleshooting
### `git clone` fails
If you don't have SSH key authentication set up on github, then clone the repo using `https`:

```bash
git clone --recurse-submodules https://github.com/matthewgiarra/trimmer.git
```
