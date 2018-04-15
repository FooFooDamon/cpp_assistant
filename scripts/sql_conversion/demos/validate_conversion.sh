cd $(dirname $0)

make || exit 1
./otl_format > otl_format.txt
./oracle_format > oracle_format.txt
./mysql_format > mysql_format.txt

echo "diff otl_format.txt oracle_format.txt"
diff otl_format.txt oracle_format.txt || exit 1

echo "diff otl_format.txt mysql_format.txt"
diff otl_format.txt mysql_format.txt || exit 1

../scripts/otl2oracle.sh otl_format.otl.sql
echo "diff otl_format.oracle.sql oracle_format.oracle.sql"
diff <(sed -n '8,$p' otl_format.oracle.sql) <(sed -n '8,$p' oracle_format.oracle.sql) || exit 1

../scripts/otl2mysql.sh otl_format.otl.sql
echo "diff otl_format.mysql.sql mysql_format.mysql.sql"
diff <(sed -n '8,$p' otl_format.mysql.sql) <(sed -n '8,$p' mysql_format.mysql.sql) || exit 1

echo -n "Clean result files? [Yes: Y or No: N]: "
read clean_flag
[ "$clean_flag" = "N" ] && exit 0
[ "$clean_flag" = "n" ] && exit 0
rm -f otl_format.txt oracle_format.txt mysql_format.txt
rm -f otl_format.oracle.sql otl_format.mysql.sql
make clean

