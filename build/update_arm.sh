#! /bin/bash

#? UPDATE 
target_user_name="pi"
target_address='192.168.0.139'
target_home="/home/pi"
target_folder="${target_home}/share"

files_home="../bin"
source_files=()

config_home="$files_home/dev"
config_files=()
# Bin Folder
for file in "$files_home"/*.elf "$files_home"/*.sh
do
  source_files+=( "$file" )
done
  scp -P 171 "${source_files[@]}" $target_user_name@$target_address:"$target_folder/bin"
#bin/dev folder
for file in "$config_home"/*
do
  config_files+=( "$file" )
done
  scp -P 171 "${config_files[@]}" $target_user_name@$target_address:"$target_folder/bin/dev"


#scp -P 171 "${source_files[@]}" $target_user_name@$target_address:"$target_folder/bin" \
#&& \
#scp -P 171 "${config_files[@]}" $target_user_name@$target_address:"$target_folder/bin/dev"