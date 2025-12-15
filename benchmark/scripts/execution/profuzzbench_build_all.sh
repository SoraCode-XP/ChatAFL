#!/bin/bash

#export NO_CACHE="--no-cache"
#export MAKE_OPT="-j4"

cd $PFBENCH
cd subjects/RTSP/Live555
docker build . -t live555 --build-arg MAKE_OPT $NO_CACHE
