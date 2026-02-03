#!/bin/bash
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install -y libc6:i386 libstdc++6:i386 zlib1g:i386
sudo apt install -y ccache