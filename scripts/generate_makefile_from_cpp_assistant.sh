#!/bin/bash

MULTI_TARGET_MAKEFILE=$(dirname $0)/../makefiles/makefile_for_multiple_targets.template
SINGLE_TARGET_MAKEFILE=$(dirname $0)/../makefiles/makefile_for_single_target.template

echo "Select the project type: 1. Multiple-targets project, 2. Single-target project"
read -p "Please enter the number: " project_type

if [ "$project_type" = "1" ]
then
	template=$MULTI_TARGET_MAKEFILE

	echo "Select the makefile type: 1. Top-makefile, 2. Sub-makefile"
	read -p "Please enter the number: " makefile_type

	if [ "$makefile_type" = "1" ]
	then
		makefile_type="Top-makefile"
	elif [ "$makefile_type" = "2" ]
	then
		makefile_type="Sub-makefile"
	else
		echo "**** Invalid makefile type: $makefile_type"
		exit 1
	fi
elif [ "$project_type" = "2" ]
then
	template=$SINGLE_TARGET_MAKEFILE
else
	echo "**** Invalid project type: $project_type"
	exit 1
fi

echo "Enter the directory where the generated makefile will be put."
read -p "Please enter the directory [will be \".\" if pressing Enter directly]: " target_dir

if [ -z "$target_dir" ]
then
	target_dir=.
fi

if [ ! -d "$target_dir" ]
then
	echo "**** Directory does not exist: $target_dir"
	exit 1
fi

if [ -f [Mm][Aa][Kk][Ee][Ff][Ii][Ll][Ee] ]
then
	mv [Mm][Aa][Kk][Ee][Ff][Ii][Ll][Ee] Makefile_`date +%Y%m%d%H%M%S`
fi

echo "#" > $target_dir/Makefile
echo "# This file was generated by \$CPP_ASSISTANT_ROOT/scripts/$(basename $0) on $(date +%Y/%m/%d)." >> $target_dir/Makefile
echo "# Edit the customized parts before it can be used." >> $target_dir/Makefile
echo "# For more help, see documents in directory \$CPP_ASSISTANT_ROOT/docs." >> $target_dir/Makefile
echo "#" >> $target_dir/Makefile

if [ "$template" = "$SINGLE_TARGET_MAKEFILE" ]
then
	cat $SINGLE_TARGET_MAKEFILE >> $target_dir/Makefile
else
	start_line=`grep "${makefile_type} begin" $MULTI_TARGET_MAKEFILE -n | awk -F : '{ print $1 }'`
	start_line=$(($start_line + 1))
	end_line=`grep "${makefile_type} end" $MULTI_TARGET_MAKEFILE -n | awk -F : '{ print $1 }'`
	end_line=$(($end_line - 1))
	sed -n "${start_line},${end_line}p" $template >> $target_dir/Makefile
fi

echo "Generation done: $target_dir/Makefile"

