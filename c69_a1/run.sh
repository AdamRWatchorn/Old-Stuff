#!bin/bash

sudo dhclient eth1
sudo git pull origin master
sudo rm test_interceptor
sudo make
sudo gcc ./test_full.c -o test_full
sudo ./test_full
