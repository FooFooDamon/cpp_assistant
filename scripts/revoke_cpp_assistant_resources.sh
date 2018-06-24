. $(dirname $0)/common_resource_actions.sh

for i in $PROTO_DIRS
do
    rm -f $USER_PROGRAM_ROOT/$i/public_protocols.proto
done

for i in $SRC_CODE_DIRS
do
    rm -rf $USER_PROGRAM_ROOT/$i/framework_*
done

for i in $CONFIG_DIRS
do
    rm -f $USER_PROGRAM_ROOT/$i/*.html
    rm -f $USER_PROGRAM_ROOT/$i/common.xml
done

find $USER_PROGRAM_ROOT -maxdepth 1 -name "*.mk" -a -type l | xargs rm -f
find $USER_PROGRAM_ROOT -maxdepth 1 -name "*.sh" -a -type l | xargs rm -f

