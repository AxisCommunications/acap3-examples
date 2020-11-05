#!/bin/bash

UBUNTU_VERSION=19.10

if [ $# -lt 1 ]; then
    echo "Argument should be: <APP_IMAGE>"
    exit 1
fi

pushd yuv || exit 1
./build.sh "$UBUNTU_VERSION"
popd || exit 1

[ -n "$REPO" ] && repo="--build-arg REPO=$REPO"
[ -n "$VERSION" ] && version="--build-arg VERSION=$VERSION"

docker build --build-arg http_proxy="${http_proxy}" --build-arg https_proxy="${https_proxy}" $repo $version --tag "$1" .

rm -rf build/
docker cp $(docker create "$1"):/opt/app ./build