mkdir -p $(dirname $0)/../logs
cd $(dirname $0)/.. || exit 1

# SERVER_COUNT and CLIENT_COUNT depend on how many server/client items are configured in the relative XML file.
# You may need to changes these if any configuration changes.
SERVER_COUNT=3
CLIENT_COUNT=3

#gnome-terminal -e 'bash -c "ulimit -c unlimited; sleep $(($RANDOM % $SERVER_COUNT)); ./tcp_server -c ./conf/servers.xml; read -p \"Press any key to continue ...\""' --tab --tab --tab
gnome-terminal --tab --tab --tab -- sleep $(($RANDOM % $SERVER_COUNT)) && ./tcp_server -c ./conf/servers.xml && read -p "Press any key to continue ..."

#gnome-terminal -e 'bash -c "ulimit -c unlimited; sleep $(($RANDOM % $CLIENT_COUNT)); ./tcp_server -c ./conf/clients.xml; read -p \"Press any key to continue ...\""' --tab --tab --tab
gnome-terminal --tab --tab --tab -- sleep $(($RANDOM % $CLIENT_COUNT)) && ./tcp_server -c ./conf/clients.xml && read -p "Press any key to continue ..."
