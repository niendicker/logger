#! /bin/bash

#? UPDATE 
target_user_name="pi"
target_address='192.168.0.139'
target_home="/home/pi"
target_folder="${target_home}/run"

project_home="./00_rpi"
files_home="${project_home}/bin"
source_files=()

config_home="$files_home/dev"
config_files=()

scripts_home="$files_home/scripts"
scripts_files=()

sql_home="$files_home/sql"
sql_files=()

# Bin Folder
for file in "$files_home"/*.arm
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

#bin/scripts folder
for file in "$scripts_home"/*
do
  scripts_files+=( "$file" )
done
  scp -P 171 "${scripts_files[@]}" $target_user_name@$target_address:"$target_folder/bin/scripts"

#bin/scripts folder
for file in "$sql_home"/*
do
  sql_files+=( "$file" )
done
  scp -P 171 "${sql_files[@]}" $target_user_name@$target_address:"$target_folder/bin/sql"


#scp -P 171 "${source_files[@]}" $target_user_name@$target_address:"$target_folder/bin" \
#&& \
#scp -P 171 "${config_files[@]}" $target_user_name@$target_address:"$target_folder/bin/dev"