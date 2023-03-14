#!/bin/bash
sudo add-apt-repository --yes ppa:gehelem/ostserver-daily
sudo add-apt-repository --yes ppa:mutlaqja/ppa
sudo apt update
sudo apt install -y libindi1 libstellarsolver ostserver ostmodules nginx gsc pip

sudo mkdir /usr/share/astrometry
sudo chmod 777 -R /usr/share/astrometry
cd /usr/share/astrometry
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-00.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-01.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-02.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-03.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-04.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-05.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-06.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-07.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-08.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-09.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-10.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4206-11.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-00.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-01.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-02.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-03.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-04.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-05.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-06.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-07.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-08.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-09.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-10.fits
#wget http://broiler.astrometry.net/~dstn/4200/index-4207-11.fits
wget http://broiler.astrometry.net/~dstn/4200/index-4208.fits
wget http://broiler.astrometry.net/~dstn/4200/index-4209.fits
wget http://broiler.astrometry.net/~dstn/4200/index-4210.fits
wget http://broiler.astrometry.net/~dstn/4200/index-4211.fits
wget http://broiler.astrometry.net/~dstn/4200/index-4212.fits
wget http://broiler.astrometry.net/~dstn/4200/index-4213.fits
wget http://broiler.astrometry.net/~dstn/4200/index-4214.fits
sudo chmod 777 -R /usr/share/astrometry

cd
cd /var/www/html
sudo wget https://github.com/gehelem/ost-front/releases/download/WorkInProgress/html.tar.gz
sudo tar -xvf html.tar.gz
sudo chmod 777 /var/www/html

cd 
sudo pip install indiweb
wget https://raw.githubusercontent.com/knro/indiwebmanager/master/indiwebmanager.service
sed -i 's/User=pi/User=ost/g' indiwebmanager.service
sudo cp indiwebmanager.service /etc/systemd/system/
sudo chmod 644 /etc/systemd/system/indiwebmanager.service
sudo systemctl daemon-reload
sudo systemctl enable indiwebmanager.service
sudo service indiwebmanager restart

cd
wget https://raw.githubusercontent.com/gehelem/OST/main/ostserver_service.sh
sudo cp ostserver_service.sh /usr/bin/

wget https://raw.githubusercontent.com/gehelem/OST/main/ostserver.service
sudo cp ostserver.service /etc/systemd/system/
sudo chmod 644 /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
sudo systemctl enable ostserver.service
sudo service ostserver restart


