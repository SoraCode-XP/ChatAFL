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

# Check for Zhipu API key if using chatafl-zp
if [[ "$FUZZER_LIST" == *"chatafl-zp"* ]] && [ -z $ZHIPU_KEY ]; then
    echo "WARNING: NO ZHIPU API KEY PROVIDED! ChatAFL-ZP may not work properly"
    echo "Please set the ZHIPU_KEY environment variable"
fi

if [[ "x$NUM_CONTAINERS" == "x" ]] || [[ "x$TIMEOUT" == "x" ]] || [[ "x$TARGET_LIST" == "x" ]] || [[ "x$FUZZER_LIST" == "x" ]]
then
    echo "Usage: $0 NUM_CONTAINERS TIMEOUT TARGET FUZZER"
    exit 1
fi

PFBENCH=$PFBENCH PATH=$PATH NUM_CONTAINERS=$NUM_CONTAINERS TIMEOUT=$TIMEOUT SKIPCOUNT=$SKIPCOUNT TEST_TIMEOUT=$TEST_TIMEOUT ZHIPU_KEY="${ZHIPU_KEY:-}" scripts/execution/profuzzbench_exec_all.sh ${TARGET_LIST} ${FUZZER_LIST}