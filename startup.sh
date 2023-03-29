#!/bin/bash

# turn off and on wifi to make sure wifi adapter works
# This assumes wifi has been previously connected to before
connmanctl disable wifi
sleep 1
connmanctl enable wifi

# install packages
sudo apt-get install -y libcurl4-openssl-dev
sudo apt install -y connman
sudo apt-get install -y libopencv-dev
sudo apt-get install -y ffmpeg

# start up camera app
path_to_app="/mnt/remote/myApps"
cd "$path_to_app"
ls

