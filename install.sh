#!/bin/bash
clear
echo ""
echo "Welcome to homected4pi installer!!!"
echo "==================================="
echo ""
echo "*** Creating directories..."
sudo chown -R $USER /bin/homected4pi
test -e /etc/homected4pi || sudo mkdir -p /etc/homected4pi
sudo chown -R $USER /etc/homected4pi
test -e /var/homected4pi || sudo mkdir -p /var/homected4pi
sudo chown -R $USER /var/homected4pi
test -e /var/www || sudo mkdir -p /var/www
sudo chown -R $USER /var/www
echo ""
echo "*** Installing libxml2..."
sudo apt-get install libxml2 libxml2-dev -q -y
echo ""
echo "*** Installing sqlite3..."
sudo apt-get install sqlite3 libsqlite3-dev -q
echo ""
echo "*** Installing FTP server..."
sudo apt-get install vsftpd -q
echo ""
echo "*** Compiling libraries..."
(cd libs; make)
echo ""
echo "*** Compiling binaries..."
(cd apps; sudo make)
echo ""