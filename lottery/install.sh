#!/bin/sh

sudo rm /etc/lotteryd.cfg 2>/dev/null
sudo cp lotteryd.cfg /etc/
sudo service lotteryd stop 2>/dev/null
sudo rm /usr/sbin/lotteryd 2>/dev/null
sudo cp bin/gcc-4.8/release/link-static/runtime-link-static/threading-multi/lotteryd /usr/sbin/ && sudo service lotteryd start
