#!/bin/bash
clear
echo "Welcome to homected4pi installer!!!"
echo "==================================="
echo ""
echo " Creating directories..."
mkdir config
mkdir logs
sudo mkdir /var/www
sudo chown -R $USER /var/www
echo ""
echo " Installing libxml2..."
sudo apt-get install libxml2 libxml2-dev -q
echo ""
echo " Installing sqlite3..."
sudo apt-get install sqlite3 sqlite3-dev -q
echo ""
echo " Installing FTP server..."
sudo apt-get install vsftpd -q
echo ""
echo " Compiling libraries..."
(cd libs; make)
echo " Compiling binaries..."
(cd apps; make)