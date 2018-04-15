mkdir ~/tmp/tcp_server_test/logs
cd ~/tmp/tcp_server_test/logs || exit 1

# SERVER_COUNT and CLIENT_COUNT depend on how many server/client items are configured in the relative XML file.
# You may need to changes these if any configuration changes.
SERVER_COUNT=3
CLIENT_COUNT=3

gnome-terminal -e 'bash -c "ulimit -c unlimited; sleep $(($RANDOM % $SERVER_COUNT)); ~/bin/tcp_server -c ~/etc/tcp_server.xml; read -p \"Press any key to continue ...\""' --tab --tab --tab

gnome-terminal -e 'bash -c "ulimit -c unlimited; sleep $(($RANDOM % $CLIENT_COUNT)); ~/bin/tcp_server -c ~/etc/tcp_client.xml; read -p \"Press any key to continue ...\""' --tab --tab --tab

