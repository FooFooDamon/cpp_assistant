#!/bin/bash

#
#  Created on: 2017-03-02
#      Author: wenxiongchang udc577@126.com
# Description: .mysql.sql --> .otl.sql
#

SRC_SQL_TYPE=mysql
DST_SQL_TYPE=otl

source $(dirname $0)/conversion_common.sh

cp $original_file $target_file

#echo "Converting ..."

for i in `seq 0 $(($COMPLEX_TYPE_COUNT - 1))`
do
	var_type=${COMPLEX_TYPES[$i]}
	sed -i "/@\([[:graph:]]*\)\(_${var_type}\)\([[:space:],;)=]\)/Is//:\1<${var_type}>\3/g" $target_file
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
			sed -i "/@\([[:graph:]]*\)\(_${sign_flag}_${len_flag}_${var_type}\)\([[:space:],;)=]\)/Is//:\1<${sign_flag} ${len_flag} ${var_type}>\3/g" $target_file
		done
		sed -i "/@\([[:graph:]]*\)\(_${len_flag}_${var_type}\)\([[:space:],;)=]\)/Is//:\1<${len_flag} ${var_type}>\3/g" $target_file
		sed -i "/@\([[:graph:]]*\)\(_${len_flag}\)\([[:space:],;)=]\)/Is//:\1<${len_flag}>\3/g" $target_file
	done
	sed -i "/@\([[:graph:]]*\)\(_${var_type}\)\([[:space:],;)=]\)/Is//:\1<${var_type}>\3/g" $target_file
done

sed -i "/@\([[:graph:]]*\)\(_char_\)\([[:digit:]]\{1,\}\)\([[:space:],;)=]\)/Is//:\1<char[\3]>\4/g" $target_file

#echo "Conversion finished successfully."

