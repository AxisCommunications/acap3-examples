#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Arguments should be: <APP_IMAGE> <UBUNTU_VERSION>"
    exit 1
fi

pushd yuv
./build.sh $2
popd

docker build --build-arg http_proxy=${http_proxy} --build-arg https_proxy=${https_proxy} --tag $1 .

rm -rf build/
docker cp $(docker create $1):/opt/app ./build
