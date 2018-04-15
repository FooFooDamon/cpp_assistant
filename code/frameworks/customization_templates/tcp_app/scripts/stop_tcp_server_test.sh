ps -ef | grep [t]cp_
kill -int `ps -ef | grep [t]cp_ | grep -v read | awk '{ print $2 }'`

