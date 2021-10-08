#!/bin/bash

#? BUILD FOR X86
project_home="00_rpi"
sources_home="$project_home/src"
include_home="$project_home/include"
#library_home="/usr/local/lib"
bin_home_dir="$project_home/bin"
bin_output="${bin_home_dir}/modbusPoll" 
sourceFiles=("")
index=0
for file in "$sources_home"/*.c
do
  sourceFiles[$index]=${file}
  ((index=index+1))
done

#Debug version 
echo arm-linux-gnueabihf-g++ -Wall -g -o "${bin_output}_dbg.arm" -I "$include_home" "${sourceFiles[@]}" && \
arm-linux-gnueabihf-g++ -Wall -g -o "${bin_output}_dbg.arm" -I "$include_home" "${sourceFiles[@]}" && \
echo "ARM: Debug version compiled succesful" 
#Production version

echo arm-linux-gnueabihf-g++ -D NDEBUG -Wall -g -o "${bin_output}.arm" -I "$include_home" "${sourceFiles[@]}" && \
arm-linux-gnueabihf-g++ -D NDEBUG -Wall -g -o "${bin_output}.arm" -I "$include_home" "${sourceFiles[@]}" && \
echo "ARM: Production version copilled successful"

