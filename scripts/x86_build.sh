#!/bin/bash

#? BUILD FOR X86
project_dir=".."
sources_dir="$project_dir/src"
include_dir="$project_dir/include"
#library_home="/usr/local/lib"
bin_output_dir="$project_dir/run"
bin_output="${bin_output_dir}/logger" 

#Load all .c files into sourceFiles variable
sourceFiles=("")
index=0
for file in "$sources_dir"/*.c
do
  sourceFiles[$index]=${file}
  ((index=index+1))
done

#Debuggin version
echo gcc -Wall -g -o "${bin_output}_dbg.bin" -I "$include_dir" "${sourceFiles[@]}"
     gcc -Wall -g -o "${bin_output}_dbg.bin" -I "$include_dir" "${sourceFiles[@]}" && \
echo "X86_Linux: Debuggin version copilled successful"
echo "------------------------------------------------"

#Profilling version 
echo gcc -Wall -pg -o "${bin_output}_prof.bin" -I "$include_dir" "${sourceFiles[@]}"
     gcc -Wall -pg -o "${bin_output}_prof.bin" -I "$include_dir" "${sourceFiles[@]}" && \
echo "X86_Linux: Profilling version copilled successful"
echo "------------------------------------------------"

#Production version - DONT DEFINE NDEBUG TO KEEP ASSERTS()
echo gcc -D QUIET_OUTPUT -Wall -o "${bin_output}.bin" -I "$include_dir" "${sourceFiles[@]}"
     gcc -D QUIET_OUTPUT -Wall -o "${bin_output}.bin" -I "$include_dir" "${sourceFiles[@]}" && \
echo "X86_Linux: Production version copilled successful"
echo "------------------------------------------------"