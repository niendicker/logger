#!/bin/bash

#? BUILD FOR ARM (RaspberryPi)
project_dir=".."
sources_dir="$project_dir/src"
include_dir="$project_dir/include"
#library_home="/usr/local/lib"
bin_output_dir="$project_dir/bin"
bin_output="${bin_output_dir}/logger" 

sourceFiles=("")
index=0
for file in "$sources_dir"/*.c
do
  sourceFiles[$index]=${file}
  ((index=index+1))
done

#Debuggin version 
echo arm-linux-gnueabihf-g++ -Wall -g -o "${bin_output}_dbg.arm" -I "$include_dir" "${sourceFiles[@]}" && \
     arm-linux-gnueabihf-g++ -Wall -g -o "${bin_output}_dbg.arm" -I "$include_dir" "${sourceFiles[@]}" -static && \
echo "ARM: Debuggin version compiled succesful" 
echo "------------------------------------------------"

#Profilling version 
echo arm-linux-gnueabihf-g++ -Wall -pg -o "${bin_output}_prof.arm" -I "$include_dir" "${sourceFiles[@]}" && \
     arm-linux-gnueabihf-g++ -Wall -pg -o "${bin_output}_prof.arm" -I "$include_dir" "${sourceFiles[@]}" && \
echo "ARM: Profilling version compiled succesful" 
echo "------------------------------------------------"

#Production version - DONT DEFINE NDEBUG TO KEEP ASSERTS()
echo arm-linux-gnueabihf-g++ -D QUIET_OUTPUT -Wall -o "${bin_output}.arm" -I "$include_dir" "${sourceFiles[@]}" && \
     arm-linux-gnueabihf-g++ -D QUIET_OUTPUT -Wall -o "${bin_output}.arm" -I "$include_dir" "${sourceFiles[@]}" && \
echo "ARM: Release version copilled successful"
echo "------------------------------------------------"
