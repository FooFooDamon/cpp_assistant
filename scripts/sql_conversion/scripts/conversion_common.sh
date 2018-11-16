#
#  Created on: 2017-03-02
#      Author: wenxiongchang udc577@126.com
# Description: Common stuff for conversions.
#

if [ "$DST_SQL_TYPE" = "cpp" ]
then
	TAIL_SUFFIX=cpp
	FULL_SUFFIX=cpp
else
	TAIL_SUFFIX=sql
	FULL_SUFFIX=$DST_SQL_TYPE.sql
fi

usage()
{
	echo "Description: $(basename $0) - Converts SQL's format, from $SRC_SQL_TYPE format to $DST_SQL_TYPE format."
	echo "Format: $(basename $0) <source SQL file name> [destination SQL/code file name]"
	echo "Examples: "
	echo "  1) $(basename $0) /home/foo/static_sqls.$SRC_SQL_TYPE.sql"
	echo "  2) $(basename $0) /home/foo/static_sqls.$SRC_SQL_TYPE.sql /home/foo/static_sqls.$FULL_SUFFIX"
}

SIGN_FLAGS=(unsigned signed)

SIGN_FLAG_COUNT=${#SIGN_FLAGS[*]}

LENGTH_FLAGS=(long short)

LENGTH_FLAG_COUNT=${#LENGTH_FLAGS[*]}

NON_CHAR_SIMPLE_TYPE_PATTERNS=([Ii][Nn][Tt] \
	[Ll][Oo][Nn][Gg] \
	[Bb][Ii][Gg][Ii][Nn][Tt] \
	[Ff][Ll][Oo][Aa][Tt] \
	[Dd][Oo][Uu][Bb][Ll][Ee] \
	[Cc][Ll][Oo][Bb] \
	[Bb][Ll][Oo][Bb] \
	[Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp])

NON_CHAR_SIMPLE_TYPES=(int \
	long \
	bigint \
	float \
	double \
	clob \
	blob \
	timestamp)

NON_CHAR_SIMPLE_TYPE_COUNT=${#NON_CHAR_SIMPLE_TYPES[*]}

COMPLEX_TYPE_PATTERNS=([Vv][Aa][Rr][Cc][Hh][Aa][Rr]_[Ll][Oo][Nn][Gg] \
	[Rr][Aa][Ww]_[Ll][Oo][Nn][Gg])

COMPLEX_TYPES=(varchar_long \
	raw_long)

COMPLEX_TYPE_COUNT=${#COMPLEX_TYPES[*]}

CHAR_TYPE_PATTERN=[Cc][Hh][Aa][Rr]

if [ $# -lt 1 ]
then
	echo "**** Source ($SRC_SQL_TYPE) SQL file missing!" >&2
	echo "See help info below for reference." >&2
	usage >&2
	exit 1
fi

if [ "$1" = "-h" ] || [ "$1" = "--help" ]
then
	usage
	exit 0
fi

original_file="$1"
#echo "Original SQL file: $original_file"

if [ -z "$(echo "$original_file" | grep "\.$SRC_SQL_TYPE\.sql$")" ]
then
	echo "**** Invalid file suffix! Suffix of the original file should be .$SRC_SQL_TYPE.sql !" >&2
	exit 1
fi

if [ ! -f "$original_file" ]
then
	echo "**** File does not exist: $original_file" >&2
	exit 1
fi

if [ $# -lt 2 ]
then
	target_file="$(dirname $original_file)/$(basename $original_file .$SRC_SQL_TYPE.sql).$FULL_SUFFIX"
else
	target_file="$2"
fi

#echo "Target SQL file: $target_file"

if [ "$DST_SQL_TYPE" = "cpp" ]
then
	target_suffix_checking_str="$(echo "$target_file" | grep "\.cpp$")"
else
	target_suffix_checking_str="$(echo "$target_file" | grep "\.$DST_SQL_TYPE\.sql$")"
fi

if [ -z "$target_suffix_checking_str" ]
then
	echo "**** Invalid file suffix! Suffix of the target file should be .$FULL_SUFFIX !" >&2
	exit 1
fi

date_str=`date +%Y%m%d%H%M%S`

if [ -f $target_file ]
then
	mv $target_file ${target_file}.old.${date_str}
fi

