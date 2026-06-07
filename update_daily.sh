#!/bin/bash
sudo apt update
sudo apt install -y ostserver ostmodules

#get ostserver front
sudo rm -rf /var/www/ostserver/*
cd /var/www/ostserver
sudo wget https://github.com/gehelem/ost-front/releases/download/WorkInProgress/html.tar.gz --no-check-certificate
sudo tar -xvf html.tar.gz
#grant access
sudo chown -R www-data:www-data /var/www/ostserver

sudo service nginx restart
sudo service ostserver restart
