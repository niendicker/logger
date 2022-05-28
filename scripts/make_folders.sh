#! /bin/bash

#? Make folder structure
target_user_name="datacollector"
target_address="192.168.1.80"
port="171"

target_home="/home/${target_user_name}"
folders="run/bin,run/bin/scripts,run/bin/dev/gc600"

ssh "${target_user_name}"@"${target_address}" "-p${port}" \
mkdir -p $target_home/{"${folders}"}

