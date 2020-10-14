if [ $# -lt 1 ]; then
    echo "Argument should be: <ENVIRONMENT_NAME>"
    exit 1
fi

docker run --gpus all -v /var/run/docker.sock:/var/run/docker.sock --network host --name "$1" -it tensorflow_to_larod /bin/bash
