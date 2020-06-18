#!/bin/sh -eu

if [ $# -lt 1 ]; then
    echo "Argument should be: <UBUNTU_VERSION>"
    exit 1
fi

# Enable building for other architectures
docker run --rm --privileged multiarch/qemu-user-static:4.2.0-6 --reset -p yes

docker build --build-arg UBUNTU_VERSION=$1 . --tag builder-yuv