#!/bin/bash

#? BUILD FOR X86
project_home="00_rpi"
sources_home="$project_home/src"
include_home="$project_home/include"
library_home="/usr/local/lib"
bin_home_dir="$project_home/bin"
bin_output="${bin_home_dir}/modbusPoll" 
sourceFiles=("")
index=0
for file in "$sources_home"/*.c
do
  sourceFiles[$index]=${file}
  ((index=index+1))
done

#Create tmp symbolic links for each header file
for headerFile in "$include_home"/*
do
  ln -s "$headerFile" "$sources_home"
done

#Compile and Link for x86 architecture
#Debug version 
echo gcc -Wall -g -o "${bin_output}_dbg.bin" -I "$include_home" -L "$library_home" "${sourceFiles[@]}"
gcc -Wall -g -o "${bin_output}_dbg.bin" -I "$include_home" -L "$library_home" "${sourceFiles[@]}"
echo "Debug version copilled successful"

#Production version 
echo gcc -Wall -g -o "${bin_output}.bin" -I "$include_home" -L "$library_home" "${sourceFiles[@]}"
gcc -D NDEBUG -Wall -g -o "${bin_output}.bin" -I "$include_home" -L "$library_home" "${sourceFiles[@]}"
echo "Production version copilled successful"

#Remove tmp header files symbolic links
for headerFile in "${sources_home}"/*.h
do
  rm "$headerFile"
done