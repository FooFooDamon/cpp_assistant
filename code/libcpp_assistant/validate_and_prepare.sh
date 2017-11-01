#!/bin/bash

# Cleans SVN "rubbish" files, e.g., foo.cpp.svn.
clean_svn_rubbish()
{
    local target_dir=$1

    find $target_dir -iname "*.c*" | xargs grep "\.svn" | xargs rm -rf
}

# More or less, this pre-check can prevent some testers doing some stupid things
# like modifying source code accidentally!
# On the other hand, if you directly edit and commit the code on the remote Linux server,
# or cooperate with colleagues working that way, this function may bring you some troubles.
# It's up to you to use it or not.
tamper_precheck()
{
    local target_dir=$1
    local SOURCE_SUFFIXES=(c C cpp cxx cc)

    for _suffix in ${SOURCE_SUFFIXES[@]}
    do
        for i in `find $target_dir -name "*.$_suffix"`
        do
            #if [ -n "`svn status $i | grep "^M"`" ]
            if [ -n "`svn status $i  2> /dev/null | grep "^M"`" ]
            then
                echo "WARNING: **** $i has been tampered! Compilation aborts!"
                exit 1
            fi
        done
    done
}

#set -x

cd $(dirname $0)
clean_svn_rubbish .
tamper_precheck .
