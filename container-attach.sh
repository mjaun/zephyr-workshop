#!/bin/bash
set -euo pipefail

# this script opens a bash terminal in the running dev container
# it must be executed in a WSL terminal

CONTAINER_ID="$(docker ps -q)"

if [[ -z "$CONTAINER_ID" ]] ; then
  echo "error: no container running!"
  exit 1
fi

docker exec \
    -ti \
    -e TERM=xterm-256color \
    -w /workdir \
    $CONTAINER_ID \
    /bin/bash

