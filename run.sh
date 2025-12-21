#!/bin/bash

PFBENCH="$PWD/benchmark"
cd $PFBENCH

PATH=$PATH:$PFBENCH/scripts/execution:$PFBENCH/scripts/analysis
NUM_CONTAINERS=$1
TIMEOUT=$(( ${2:-1440} * 60))
SKIPCOUNT="${SKIPCOUNT:-1}"
TEST_TIMEOUT="${TEST_TIMEOUT:-5000}"

export TARGET_LIST=$3
export FUZZER_LIST=$4

# Process named parameters
while [[ $# -gt 4 ]]; do
    case "$5" in
        --prefix=*)
            export IMAGE_PREFIX="${5#*=}"
            shift
            ;;
        --prefix)
            export IMAGE_PREFIX="$6"
            shift 2
            ;;
        *)
            # Unknown option
            shift
            ;;
    esac
done

# If IMAGE_PREFIX is not set via named parameters, determine based on API key
if [ -z "$IMAGE_PREFIX" ]; then
    if [ ! -z $ZHIPU_KEY ]; then
        export IMAGE_PREFIX="chataflzp-"
    else
        export IMAGE_PREFIX=""
    fi
fi

if [[ "x$NUM_CONTAINERS" == "x" ]] || [[ "x$TIMEOUT" == "x" ]] || [[ "x$TARGET_LIST" == "x" ]] || [[ "x$FUZZER_LIST" == "x" ]]
then
    echo "Usage: $0 NUM_CONTAINERS TIMEOUT TARGET FUZZER [--prefix=PREFIX]"
    echo "Example: $0 1 1440 live555 chatafl-zp --prefix=chataflzp-"
    exit 1
fi

PFBENCH=$PFBENCH PATH=$PATH NUM_CONTAINERS=$NUM_CONTAINERS TIMEOUT=$TIMEOUT SKIPCOUNT=$SKIPCOUNT TEST_TIMEOUT=$TEST_TIMEOUT IMAGE_PREFIX=$IMAGE_PREFIX scripts/execution/profuzzbench_exec_all.sh ${TARGET_LIST} ${FUZZER_LIST}