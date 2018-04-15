#!/bin/bash

#
#  Created on: 2017-03-02
#      Author: wenxiongchang wenxiongchang@revenco.com
# Description: .otl.sql --> .cpp
#

SRC_SQL_TYPE=otl
DST_SQL_TYPE=cpp

source $(dirname $0)/conversion_common.sh

#target_h=$(basename "$target_file" .cpp).h
target_h=$(dirname "$target_file")/static_sqls.h
if [ -f "$target_h" ]
then
	echo "** WARNING: Found $target_h already existed, had renamed it to ${target_h}.old.${date_str}" >&2
	mv "$target_h" "${target_h}.old.${date_str}"
fi
cp $(dirname $0)/static_sqls.h "$target_h"

#echo "Converting ..."

grep --color=auto "^/\*.*STATIC_SQL" "$original_file" -n | sort -n > sql_names.txt

print_func_file=print_function.txt

echo "void print_static_sql_definitions(void)" > $print_func_file
echo "{" >> $print_func_file

echo "/*" > $target_file
echo " * This file was generated from $(basename $original_file) by $(basename $0) on $(date +%Y/%m/%d)." >> $target_file
echo " * DO NOT EDIT it!!!" >> $target_file
echo "*/" >> $target_file
echo "" >> $target_file

echo "#include <stdio.h>" >> $target_file
echo "" >> $target_file

echo "#include \"$target_h\"" >> $target_file
echo "" >> $target_file

line_num=0
begin_line_num=0
begin_line_name=""
begin_line_str=""
end_line_num=0
end_line_name=""
end_line_str=""

while read line_str
do
	line_num=$(($line_num + 1))
	if [ $(($line_num % 2)) -eq 1 ]
	then
		begin_line_str="$line_str"
		begin_line_name=$(echo "$begin_line_str" | awk '{ print $2 }' | tr -d "(|)" | sed "/STATIC_SQL/s///g")
		begin_line_num=$(echo "$begin_line_str" | awk -F : '{ print $1 }')
		#echo "Before writting: begin line string: [$begin_line_str], begin line number: $begin_line_num, name: $begin_line_name"
		continue
	else
		end_line_str="$line_str"
		end_line_name=$(echo "$end_line_str" | awk '{ print $2 }' | tr -d "(|)" | sed "/STATIC_SQL/s///g")
		end_line_num=$(echo "$end_line_str" | awk -F : '{ print $1 }')
		if [ "$end_line_name" != "$begin_line_name" ] || [ $end_line_num -le $begin_line_num ]
		then
			echo "**** Invalid end line contents [$end_line_str], line name unmatched, or line number too small" >&2
			echo "**** Nearest begin line contents: $begin_line_str" >&2
			exit 1
		fi
		#echo "Before writting: end line string: [$end_line_str], end line number: $end_line_num, name: $end_line_name"
		tmp_file=STATIC_SQL_${end_line_name}.cpp
		#echo "On writting: begin line string: [$begin_line_str], end line string: [$end_line_str]"
		#echo "On writting: begin line number: $(($begin_line_num + 1)), end line number: $(($end_line_num - 1)), temporary file: $tmp_file"
		sed -n "$(($begin_line_num + 1)),$(($end_line_num - 1))p" $original_file > $tmp_file
		sed -i "/\r/s///g" $tmp_file
		sed -i '/^[[:space:]]*$/d' $tmp_file
		sed -i '/^$/d' $tmp_file
		end_line_num=$(wc -l $tmp_file | awk '{ print $1 }')
		sed -i "${end_line_num}s/\;[[:space:]]*$//g" $tmp_file
		sed -i "/^/s//    \"&/g" $tmp_file
		sed -i "/$/s//&\\\n\"/g" $tmp_file
		#sed -i "1iconst char* STATIC_SQL_VAR\($end_line_name\) =" $tmp_file
		sed -i "1s/^    //g; 1s/^/const char* STATIC_SQL_VAR\($end_line_name\) = &/g" $tmp_file
		end_line_num=$(wc -l $tmp_file | awk '{ print $1 }')
		sed -i "${end_line_num}s/\\\n\"$/\";/g" $tmp_file
		cat $tmp_file >> $target_file
		echo "" >> $target_file
		rm -f $tmp_file
	
		echo "    printf(\"$(echo "$begin_line_str" | awk -F : '{ print $2 }')\n\n\");" >> $print_func_file
		echo "    printf(\"Actual variable in source code: %s\n\n\", STATIC_SQL_VAR_NAME($end_line_name));" >> $print_func_file
		echo "    printf(\"SQL contents in OTL format:\n\n%s\n\n\", STATIC_SQL_VAR($end_line_name));" >> $print_func_file
		echo "    printf(\"$(echo "$end_line_str" | awk -F : '{ print $2 }')\n\n\n\n\n\");" >> $print_func_file
		echo "" >> $print_func_file
	fi
done < sql_names.txt

echo "}" >> $print_func_file
sed -i "/\r/s///g" $print_func_file
cat $print_func_file >> $target_file

rm -f $print_func_file
rm -f sql_names.txt

echo "" >> $target_file
#echo "Conversion finished successfully."

