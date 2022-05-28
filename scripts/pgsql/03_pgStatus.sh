#!/bin/bash

#Linux and MAC OSx
#sudo ps auxwww | grep postgres --color 
#sudo systemctl | grep postgres --color
#sudo systemctl status postgresql*

#Using the shell script provided by postgres to do that
sudo /etc/init.d/postgresql status


