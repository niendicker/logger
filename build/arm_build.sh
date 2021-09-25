#!/bin/bash
#? BUILD
sources_home_dir="../src"

sourceFiles=("")
index=0
for file in "$sources_home_dir"/*.c
do
  sourceFiles[$index]=${file}
  ((index=index+1))
done

include_lib_dir=("/usr/local/lib")
include_dir="../include"

bin_home_dir="../bin"
bin_output="${bin_home_dir}/modbusPoll" 

#Create tmp symbolic links for each header file
for headerFile in "$include_dir"/*
do
  ln -s "${headerFile}" "${sources_home_dir}"
done

#Compile and Link for arm architecture
#Debug version 
echo arm-linux-gnueabihf-g++ -Wall -g -o "${bin_output}_dbg.elf" -I "${include_dir[@]}" -L "${include_lib_dir[@]}"   "${sourceFiles[@]}" && \
arm-linux-gnueabihf-g++ -Wall -g -o "${bin_output}_dbg.elf" -I "${include_dir[@]}" -L "${include_lib_dir[@]}"   "${sourceFiles[@]}" && \
echo "Debug version compiled succesful" 
#Production version
echo arm-linux-gnueabihf-g++ -D NDEBUG -Wall -o "${bin_output}.elf" -I "${include_dir[@]}" -L "${include_lib_dir[@]}"   "${sourceFiles[@]}" && \
arm-linux-gnueabihf-g++ -D NDEBUG -Wall -o "${bin_output}.elf" -I "${include_dir[@]}" -L "${include_lib_dir[@]}"   "${sourceFiles[@]}" && \
echo "Production version copilled successful"

#Remove tmp header files symbolic links
for headerFile in "${sources_home_dir}"/*.h
do
  rm "$headerFile"
done
