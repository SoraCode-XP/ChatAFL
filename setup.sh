#!/bin/bash

# Parse command line arguments
BUILD_TARGET=""
NO_CACHE=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-cache)
            NO_CACHE=1
            shift
            ;;
        *)
            BUILD_TARGET="$1"
            shift
            ;;
    esac
done

if [ -z $KEY ] && [ -z $ZHIPU_KEY ]; then
    echo "NO API KEY PROVIDED! Please set either the KEY (OpenAI) or ZHIPU_KEY (Zhipu AI) environment variable"
    exit 0
fi

if [ ! -z $KEY ]; then
    echo "OpenAI API key provided"
fi

if [ ! -z $ZHIPU_KEY ]; then
    echo "Zhipu AI API key provided"
fi

if [ ! -z $BUILD_TARGET ]; then
    echo "Building only target: $BUILD_TARGET"
fi

# Update the openAI key if provided
if [ ! -z $KEY ]; then
  for x in ChatAFL ChatAFL-CL1 ChatAFL-CL2;
do
  sed -i "s/#define OPENAI_TOKEN \".*\"/#define OPENAI_TOKEN \"$KEY\"/" $x/chat-llm.h
done
fi

# Check and update Zhipu API key if provided
if [ ! -z $ZHIPU_KEY ]; then
    echo "Setting Zhipu API key..."
    sed -i "s/#define ZHIPU_TOKEN \".*\"/#define ZHIPU_TOKEN \"$ZHIPU_KEY\"/" ChatAFL-ZP/chat-llm.h
    echo "Zhipu API key updated successfully!"
else
    echo "No Zhipu API key provided. You can set it later using the ZHIPU_KEY environment variable."
fi

# Set the image prefix based on which API key is provided
if [ ! -z $ZHIPU_KEY ]; then
    export IMAGE_PREFIX="chataflzp-"
else
    export IMAGE_PREFIX=""
fi

# Copy the different versions of ChatAFL to the benchmark directories
for subject in ./benchmark/subjects/*/*; do
  rm -r $subject/aflnet 2>&1 >/dev/null
  cp -r aflnet $subject/aflnet

  rm -r $subject/chatafl 2>&1 >/dev/null
  cp -r ChatAFL $subject/chatafl
  
  rm -r $subject/chatafl-cl1 2>&1 >/dev/null
  cp -r ChatAFL-CL1 $subject/chatafl-cl1
  
  rm -r $subject/chatafl-cl2 2>&1 >/dev/null
  cp -r ChatAFL-CL2 $subject/chatafl-cl2

  rm -r $subject/chatafl-zp 2>&1 >/dev/null
  cp -r ChatAFL-ZP $subject/chatafl-zp
done;

# Build the docker images

PFBENCH="$PWD/benchmark"
cd $PFBENCH
PFBENCH=$PFBENCH IMAGE_PREFIX=$IMAGE_PREFIX BUILD_TARGET=$BUILD_TARGET NO_CACHE=$NO_CACHE scripts/execution/profuzzbench_build_all.sh