#!/bin/bash

#
#  Created on: 2017-03-02
#      Author: wenxiongchang wenxiongchang@revenco.com
# Description: .otl.sql --> .mysql.sql
#

SRC_SQL_TYPE=otl
DST_SQL_TYPE=mysql

source $(dirname $0)/conversion_common.sh

cp $original_file $target_file

#echo "Converting ..."

sed -i "/:/s//@/g" $target_file

for i in `seq 0 $(($COMPLEX_TYPE_COUNT - 1))`
do
	var_type=${COMPLEX_TYPES[$i]}
	sed -i "/<[[:space:]]*${var_type}[[:space:]]*>/Is//_${var_type}/g" $target_file
done

for i in `seq 0 $(($NON_CHAR_SIMPLE_TYPE_COUNT - 1))`
do
	var_type=${NON_CHAR_SIMPLE_TYPES[$i]}
	for j in `seq 0 $(($LENGTH_FLAG_COUNT - 1))`
	do
		len_flag=${LENGTH_FLAGS[$j]}
		for k in `seq 0 $(($SIGN_FLAG_COUNT - 1))`
		do
			sign_flag=${SIGN_FLAGS[$k]}
			sed -i "/<[[:space:]]*${sign_flag}[[:space:]]\{1,\}${len_flag}[[:space:]]\{1,\}${var_type}[[:space:]]*>/Is//_${sign_flag}_${len_flag}_${var_type}/g" $target_file
		done
		sed -i "/<[[:space:]]*${len_flag}[[:space:]]\{1,\}${var_type}[[:space:]]*>/Is//_${len_flag}_${var_type}/g" $target_file
		sed -i "/<[[:space:]]*${len_flag}[[:space:]]*>/Is//_${len_flag}/g" $target_file
	done
	sed -i "/<[[:space:]]*${var_type}[[:space:]]*>/Is//_${var_type}/g" $target_file
done

sed -i "/<[[:space:]]*char\[/Is//_char_/g; /\][[:space:]]*>/s///g" $target_file

#echo "Conversion finished successfully."

