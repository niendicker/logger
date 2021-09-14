#! /bin/bash

source="hello.arm.dl"
destination="/home/pi/bin/"

username="pi"
addr='192.168.0.139'


scp -P 171 $source $username@$addr:$destination