#!/bin/bash
sudo apt-get install software-properties-common
sudo add-apt-repository --yes ppa:gehelem/ostserver
sudo add-apt-repository --yes ppa:mutlaqja/ppa
sudo apt update
sudo apt upgrade -y
sudo apt install -y libindi1 libstellarsolver ostserver ostmodules nginx gsc pip indi-playerone indi-asi

sudo mkdir /usr/share/astrometry
sudo chown -R $USER /usr/share/astrometry
cd /usr/share/astrometry
#wget http://data.astrometry.net/4200/index-4206-00.fits
#wget http://data.astrometry.net/4200/index-4206-01.fits
#wget http://data.astrometry.net/4200/index-4206-02.fits
#wget http://data.astrometry.net/4200/index-4206-03.fits
#wget http://data.astrometry.net/4200/index-4206-04.fits
#wget http://data.astrometry.net/4200/index-4206-05.fits
#wget http://data.astrometry.net/4200/index-4206-06.fits
#wget http://data.astrometry.net/4200/index-4206-07.fits
#wget http://data.astrometry.net/4200/index-4206-08.fits
#wget http://data.astrometry.net/4200/index-4206-09.fits
#wget http://data.astrometry.net/4200/index-4206-10.fits
#wget http://data.astrometry.net/4200/index-4206-11.fits
#wget http://data.astrometry.net/4200/index-4207-00.fits
#wget http://data.astrometry.net/4200/index-4207-01.fits
#wget http://data.astrometry.net/4200/index-4207-02.fits
#wget http://data.astrometry.net/4200/index-4207-03.fits
#wget http://data.astrometry.net/4200/index-4207-04.fits
#wget http://data.astrometry.net/4200/index-4207-05.fits
#wget http://data.astrometry.net/4200/index-4207-06.fits
#wget http://data.astrometry.net/4200/index-4207-07.fits
#wget http://data.astrometry.net/4200/index-4207-08.fits
#wget http://data.astrometry.net/4200/index-4207-09.fits
#wget http://data.astrometry.net/4200/index-4207-10.fits
#wget http://data.astrometry.net/4200/index-4207-11.fits
wget http://data.astrometry.net/4200/index-4208.fits
wget http://data.astrometry.net/4200/index-4209.fits
wget http://data.astrometry.net/4200/index-4210.fits
wget http://data.astrometry.net/4200/index-4211.fits
wget http://data.astrometry.net/4200/index-4212.fits
wget http://data.astrometry.net/4200/index-4213.fits
wget http://data.astrometry.net/4200/index-4214.fits
sudo chmod 744 -R /usr/share/astrometry/

cd
cd /var/www/html
sudo wget https://github.com/gehelem/ost-front/releases/download/WorkInProgress/html.tar.gz --no-check-certificate
sudo tar -xvf html.tar.gz
sudo chmod -R +rX /var/www/html
sudo chown -R $USER /var/www/html


cd 
sudo apt-get install pipenv
mkdir indiweb
cd indiweb
pipenv --python=`which python3`
pipenv install bottle==0.12.25
pipenv install indiweb
pipenv install importlib_metadata

cd
echo [Unit]                                  >> indiwebmanager.service
echo Description=INDI Web Manager >> indiwebmanager.service
echo After=multi-user.target >> indiwebmanager.service
echo  >> indiwebmanager.service
echo [Service] >> indiwebmanager.service
echo Type=idle >> indiwebmanager.service
echo # MUST SET YOUR USERNAME HERE. >> indiwebmanager.service
echo User=$USER >> indiwebmanager.service
echo WorkingDirectory=/home/$USER/indiweb  >> indiwebmanager.service
echo ExecStart=/usr/bin/pipenv run indi-web -v >> indiwebmanager.service
echo Restart=always >> indiwebmanager.service
echo RestartSec=5 >> indiwebmanager.service
echo  >> indiwebmanager.service
echo [Install] >> indiwebmanager.service
echo WantedBy=multi-user.target >> indiwebmanager.service

sudo cp indiwebmanager.service /etc/systemd/system/
sudo chmod 644 /etc/systemd/system/indiwebmanager.service
sudo systemctl daemon-reload
sudo systemctl enable indiwebmanager.service
sudo service indiwebmanager restart

cd
wget https://raw.githubusercontent.com/gehelem/OST/main/ostserver_service.sh --no-check-certificate
sudo cp ostserver_service.sh /usr/bin/

wget https://raw.githubusercontent.com/gehelem/OST/main/ostserver.service --no-check-certificate
sed -i "s/ostusername/"$USER"/g" ostserver.service
sudo cp ostserver.service /etc/systemd/system/
sudo chmod 644 /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
sudo systemctl enable ostserver.service
sudo service ostserver restart


