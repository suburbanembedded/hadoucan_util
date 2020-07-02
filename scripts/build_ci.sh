#!/usr/bin/env bash

trap 'exit -1' err

set -v

CONTAINER_ID=$(docker create -v $GITHUB_WORKSPACE:/tmp/workspace -it docker.pkg.github.com/suburbanembedded/hadoucan_util/hadoucan_util_tools:${GITHUB_REF##*/}  /bin/bash)
docker start $CONTAINER_ID
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID ./generate_cmake.sh
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/debug/
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/release/
docker stop $CONTAINER_ID
