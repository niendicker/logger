#!/bin/bash

#? BUILD
INCLUDE="-I/home/dev/dbms/00_rpi/lib/modbus/src -I/usr/local/include"
LIB_INCLUDE="-L/usr/local/lib -lwiringPi"
#C_SOURCES="main"
#PROJECT_FOLDER="/home/dev/dbms/00_rpi"
#ls "${PROJECT_FOLDER}/*.c" > "C_SOURCES" 
C_SOURCES=("modbus_master_tcp.c" "slave_config_parser.c")
BIN_NAME="modbusMasterTcp"
BIN_OUTPUT="${BIN_NAME}"
#.${BIN_TYPE}"

#echo "${C_SOURCES[@]}"

#Compile
#Link
#Clear object files
arm-linux-gnueabihf-g++ "$INCLUDE" "$LIB_INCLUDE" "${C_SOURCES[@]}" -o "${BIN_OUTPUT}"