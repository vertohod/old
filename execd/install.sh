#!/bin/sh

sudo rm /etc/execd.cfg 2>/dev/null
sudo cp execd.cfg /etc/
sudo service execd stop 2>/dev/null
sudo rm /usr/sbin/execd 2>/dev/null
sudo cp bin/gcc-4.8/release/link-static/runtime-link-static/threading-multi/execd /usr/sbin/ && sudo service execd start
