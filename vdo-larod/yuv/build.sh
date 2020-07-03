#!/bin/sh -e

if [ $# -lt 1 ]; then
    echo "Argument should be: <UBUNTU_VERSION>"
    exit 1
fi

# Enable building for other architectures
docker run --rm --privileged multiarch/qemu-user-static:4.2.0-6 --reset -p yes

docker build --build-arg http_proxy="${http_proxy}" --build-arg https_proxy="${https_proxy}" --build-arg UBUNTU_VERSION="$1" . --tag builder-yuv:1.0-ubuntu"$1"
