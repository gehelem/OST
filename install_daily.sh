#!/bin/bash
sudo apt-get install software-properties-common
sudo add-apt-repository --yes ppa:gehelem/ostserver-daily
sudo add-apt-repository --yes ppa:mutlaqja/ppa
sudo apt update
sudo apt upgrade -y
sudo apt install -y libindi1 libstellarsolver ostserver ostmodules nginx gsc pip indi-playerone indi-asi

sudo mkdir /usr/share/astrometry
sudo chmod 777 -R /usr/share/astrometry
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
sudo chmod 777 -R /usr/share/astrometry

######################################################### NGINX CONF
#create local media folder
cd
#rm -Rf ostmedia
mkdir ostmedia
#grant access
sudo usermod -a -G $USER www-data

#adapt nginx conf
rm ostserver.nginx
wget https://raw.githubusercontent.com/gehelem/OST/main/ostserver.nginx --no-check-certificate
sed -i "s/ostusername/"$USER"/g" ostserver.nginx
sudo cp ostserver.nginx /etc/nginx/sites-available
cd /etc/nginx/sites-enabled
sudo rm default
sudo rm ostserver.nginx
sudo ln -s /etc/nginx/sites-available/ostserver.nginx

#get ostserver front
cd
sudo rm -Rf /var/www/ostserver
sudo mkdir /var/www/ostserver
cd /var/www/ostserver
sudo wget https://github.com/gehelem/ost-front/releases/download/WorkInProgress/html.tar.gz --no-check-certificate
sudo tar -xvf html.tar.gz
#grant access
sudo chown -R www-data:www-data /var/www/ostserver

sudo service nginx restart



# INDIWEBMANAGER
cd 
sudo apt-get install -y pipenv
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

#OSTSERVER SERVICE
cd
rm ostserver_service.sh 
wget https://raw.githubusercontent.com/gehelem/OST/main/ostserver_service.sh --no-check-certificate
sed -i "s/ostusername/"$USER"/g" ostserver_service.sh
sudo rm /usr/bin/ostserver_service.sh 
sudo cp ostserver_service.sh /usr/bin/

cd
rm ostserver.service
wget https://raw.githubusercontent.com/gehelem/OST/main/ostserver.service --no-check-certificate
sed -i "s/ostusername/"$USER"/g" ostserver.service
sudo cp ostserver.service /etc/systemd/system/
sudo chmod 644 /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
sudo systemctl enable ostserver.service
sudo service ostserver restart


