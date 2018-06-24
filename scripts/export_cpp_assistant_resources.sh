#!/bin/bash

. $(dirname $0)/common_resource_actions.sh

ln -s -f $CPP_ASSISTANT_ROOT/makefiles/*.mk $USER_PROGRAM_ROOT
ln -s -f $CPP_ASSISTANT_ROOT/code/libcpp_assistant/validate_and_prepare.sh $USER_PROGRAM_ROOT

for _dir_ in $CONFIG_DIRS
do
    ln -s -f $CPP_ASSISTANT_ROOT/conf/common.xml $USER_PROGRAM_ROOT/$_dir_
    ln -s -f $CPP_ASSISTANT_ROOT/conf/*.html $USER_PROGRAM_ROOT/$_dir_
done

for _dir_ in $SRC_CODE_DIRS
do
	if [ -d $USER_PROGRAM_ROOT/$_dir_/framework_public ]
	then
		continue
	fi

	for _target_ in framework_public $FRAMEWORK
	do
		if [ "$_target_" = "framework_public" ]
		then
        	src_framework_item=$CPP_ASSISTANT_ROOT/code/frameworks/framework_public
		else
        	src_framework_item=$CPP_ASSISTANT_ROOT/code/frameworks/framework_templates/$_target_
		fi
        dst_framework_item=$USER_PROGRAM_ROOT/$_dir_/$_target_
		mkdir -p $dst_framework_item
		echo "$dst_framework_item created"
        # NOTE: the directory to be searched must end with / in order to filter out itself with grep
        for i in `find $src_framework_item/ -type d | grep -v '/$'`
        do
           	src_dir=$i
           	dst_dir=${src_dir/$src_framework_item/$dst_framework_item}
           	mkdir -p $dst_dir
           	echo "$dst_dir created"
        done
           	
		for i in `find $src_framework_item/ -type f`
        do
			src_file=$i
			dst_file=${src_file/$src_framework_item/$dst_framework_item}
			ln -s -f $src_file $dst_file
			echo "$src_file -> $dst_file"
		done

		if [ $_target_ = $FRAMEWORK ]
		then
			mv $dst_framework_item $USER_PROGRAM_ROOT/$_dir_/framework_core
		fi
	done
done

for _dir_ in $PROTO_DIRS
do
    ln -s -f $CPP_ASSISTANT_ROOT/code/frameworks/framework_public/public_protocols.proto $USER_PROGRAM_ROOT/$_dir_
done

