#!/bin/sh -e

[ $# -eq 1 ] || {
    echo "Argument should be: <APP_IMAGE>"
    exit 1
}

docker build --build-arg http_proxy="${http_proxy:-}" --build-arg https_proxy="${https_proxy:-}" --tag "$1" .

rm -rf result/ build/
# shellcheck disable=SC2046 # Docker container ID never needs to have quotes
docker cp $(docker create "$1":latest):/build/src/ build
