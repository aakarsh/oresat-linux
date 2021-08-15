#!/bin/bash

# user ubuntu:latest armhf image digest as of Aug 15th 2021
sudo docker run --privileged --user root --platform linux/arm/v7 -v $(pwd):/oresat-linux -w /oresat-linux ubuntu@sha256:d1289429d09bf5ce27d87ecb83b391201dfbb71434794bffde98cef5b400dcc6 sh -c 'apt update && apt install -y zstd device-tree-compiler git make cpp m4 gcc sudo dosfstools git kpartx wget parted && ls -la && ./build_images.sh generic'
