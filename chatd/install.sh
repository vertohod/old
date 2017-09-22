#!/bin/sh

sudo rm -rf /tmp/files 2>/dev/null
sudo mkdir /tmp/files
sudo rm /etc/chatd.cfg && sudo cp chatd.cfg /etc/
sudo service chatd stop && sudo rm /usr/sbin/chatd
sudo cp bin/gcc-4.8/debug/link-static/runtime-link-static/threading-multi/chatd /usr/sbin/ && sudo service chatd start
