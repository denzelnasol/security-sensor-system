#!/bin/bash

# install packages
sudo apt-get install -y libcurl4-openssl-dev
sudo apt install -y connman

# turn off and on wifi to make sure wifi adapter works
connmanctl disable wifi
sleep 1
connmanctl enable wifi

# start up camera app
path_to_app="/mnt/remote/myApps"
cd "$path_to_app"
ls

