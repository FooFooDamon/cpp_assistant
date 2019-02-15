#!/bin/bash

SCRIPT_DIR=$(dirname $0)
version=`grep ".*, [0-9]\{4,\}[/-][0-9][0-9]\{1,2\}[/-][0-9]\{1,2\}, v.*" $SCRIPT_DIR/../docs/changelog.txt | head -1 | awk '{ print $3 }' | awk -F : '{ print $1 }'`
make clean -C $SCRIPT_DIR/../code/libcpp_assistant
make clean -C $SCRIPT_DIR/../code/libcpp_assistant/src/unittest
make update_copyright -C $SCRIPT_DIR/../code/libcpp_assistant
for i in `find $SCRIPT_DIR/../code/frameworks/ -name "*.cpp" -o -name "*.h" | grep -v "\.svn" | grep -v "\.git"`
do
	update_code_copyright.sh $i
done
# tts.sh is a self-made script that turns all tabs in files into spaces.
tts.sh $SCRIPT_DIR/../code
tar -zcvf ~/cpp_assistant_${version}.tar.gz $SCRIPT_DIR/../../cpp_assistant/ \
	--exclude ".git" --exclude ".svn" --exclude "*.log" --exclude "*.a" --exclude "*.so" --exclude "*.so.*"
nautilus ~/cpp_assistant_${version}.tar.gz

