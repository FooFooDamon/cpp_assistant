mkdir -p $(dirname $0)/../logs
cd $(dirname $0)/.. || exit 1

for i in server client
do
	config_file=./conf/${i}s.xml
	section_start_line=`grep "<identities>" $config_file -n | awk -F : '{ print $1 }'`
	section_end_line=`grep "</identities>" $config_file -n | awk -F : '{ print $1 }'`
	instance_count=`sed -n "${section_start_line},${section_end_line}p" "$config_file" | grep -i "enabled=\"yes\"" -c`

	for j in `seq 1 $instance_count`
	do
		# Does not work
		#gnome-terminal -- $(dirname $0)/_run_single_instance.sh $instance_count $config_file

		gnome-terminal -- bash -c "$(dirname $0)/_run_single_instance.sh $instance_count $config_file"
	done
done

