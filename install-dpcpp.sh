#!/bin/sh

# Prerequisite: recent Ubuntu environment

echo Add Intel Apt repository

wget -qO- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB | sudo tee /etc/apt/trusted.gpg.d/GPG-PUB-KEY-INTEL-SW-PRODUCTS.asc
echo "deb https://apt.repos.intel.com/oneapi all main" | sudo tee /etc/apt/sources.list.d/oneAPI.list
sudo apt update -q
        
echo Install Intel oneAPI

sudo apt install -q -y intel-oneapi-compiler-dpcpp-cpp

echo Done
