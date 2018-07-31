#set -o functrace
#trap 'echo -ne "\e]0;$BASH_COMMAND\007"' DEBUG
#PS1="\e]0;\s\007$PS1"
ulimit -c unlimited
sleep $(($RANDOM % $1))
$(dirname $0)/../tcp_server -c "$2"
read -p "Press any key to continue ..."

