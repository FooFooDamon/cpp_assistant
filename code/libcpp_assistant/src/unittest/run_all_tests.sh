TEST_DIR=$(cd $(dirname $0) && pwd)
echo "Test directory: $TEST_DIR"
export LD_LIBRARY_PATH=$TEST_DIR/../..
cd $TEST_DIR
rm -f failed_cases.txt
for i in `file $TEST_DIR/* | grep -v "\.o\|\.cpp" | grep -i "executable\|ELF.*LSB" | awk -F : '{ print $1 }'`
do
    echo "Running test case: $i ..."
    time $i
    if [ $? -ne 0 ]
    then
        echo $i >> failed_cases.txt
        break
    fi
done

