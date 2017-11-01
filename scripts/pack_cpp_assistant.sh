#!/bin/bash

SCRIPT_DIR=$(dirname $0)
version=`head -1 $SCRIPT_DIR/../docs/changelog.txt | awk '{ print $3 }' | awk -F : '{ print $1 }'`
make clean -C $SCRIPT_DIR/../code/libcpp_assistant
make clean -C $SCRIPT_DIR/../code/libcpp_assistant/src/unittest
make add_copyright -C $SCRIPT_DIR/../code/libcpp_assistant
# tts.sh is a self-made script that turns all tabs in files into spaces.
tts.sh $SCRIPT_DIR/../code
tar -zcvf ~/cpp_assistant_${version}.tar.gz $SCRIPT_DIR/../../cpp_assistant/
nautilus ~/cpp_assistant_${version}.tar.gz

