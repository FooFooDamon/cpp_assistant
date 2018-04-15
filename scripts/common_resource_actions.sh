#!/bin/bash

usage()
{
    local script_name=$(basename $0)
	local framework=tcp_server
    local CODE_ROOT_EXAMPLE=/data/wxc/tscode/src
    local USER_PROGRAM_ROOT_EXAMPLE=$CODE_ROOT_EXAMPLE/fund_adjust_requester
    echo "usage: $script_name <framework name> <user program root directory> [<configuration sub-directories>] [<module source code sub-directories>] [<protocol sub-directories>]"
    echo "example 1: $script_name $framework $USER_PROGRAM_ROOT_EXAMPLE"
    echo "example 2: $script_name $framework $USER_PROGRAM_ROOT_EXAMPLE conf"
    echo "example 3: $script_name $framework $USER_PROGRAM_ROOT_EXAMPLE conf src"
    echo "example 4: $script_name $framework $USER_PROGRAM_ROOT_EXAMPLE conf src src/operator"
    echo "example 5: $script_name $framework $CODE_ROOT_EXAMPLE/stress_test conf \"src/server src/client\" src/protocols"
}

CPP_ASSISTANT_ROOT="$(dirname $0)/.."

if [ $# -lt 2 ]
then
    usage
    exit 1
fi

FRAMEWORK="$1"
if [ ! -d $CPP_ASSISTANT_ROOT/code/frameworks/framework_templates/"$FRAMEWORK" ]
then
    echo "**** framework [$FRAMEWORK] not found"
    exit 1
fi

USER_PROGRAM_ROOT="$2"
if [ ! -d "$USER_PROGRAM_ROOT" ]
then
    echo "**** user program root directory [$USER_PROGRAM_ROOT] not found"
    exit 1
fi

if [ $# -gt 2 ]
then
    CONFIG_DIRS="$3"
else
    CONFIG_DIRS="conf"
fi

for i in $CONFIG_DIRS
do
    if [ ! -d "$USER_PROGRAM_ROOT/$i" ]
    then
        echo "**** configuration directory [$USER_PROGRAM_ROOT/$i] not found"
        usage
        exit 1
    fi
done

if [ $# -gt 3 ]
then
    SRC_CODE_DIRS="$4"
else
    SRC_CODE_DIRS="src"
fi

for i in $SRC_CODE_DIRS
do
    if [ ! -d "$USER_PROGRAM_ROOT/$i" ]
    then
        echo "**** module source code directory [$USER_PROGRAM_ROOT/$i] not found"
        usage
        exit 1
    fi
done

if [ $# -gt 4 ]
then
    PROTO_DIRS="$5"
else
    PROTO_DIRS="src/operator"
fi

for i in $PROTO_DIRS
do
    if [ ! -d "$USER_PROGRAM_ROOT/$i" ]
    then
        echo "**** protocol directory [$USER_PROGRAM_ROOT/$i] not found"
        usage
        exit 1
    fi
done

