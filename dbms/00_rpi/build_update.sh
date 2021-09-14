#! /bin/bash

#? BUILD
INCLUDE="-I/usr/local/include"
LIB_INCLUDE="-L/usr/local/lib -lwiringPi"
#C_SOURCES="main"
#PROJECT_FOLDER="/home/dev/dbms/00_rpi"
#ls "${PROJECT_FOLDER}/*.c" > "C_SOURCES" 
C_SOURCES=(\
"modbus_master_tcp.c" \
"slave_config_parser.c" )
#"./lib/modbus/src/modbus.c" \
#"./lib/modbus/src/modbus-data.c" \
#"./lib/modbus/src/modbus-rtu.c" \
#"./lib/modbus/src/modbus-tcp.c" \
#)

BIN_NAME="modbusMasterTcp"
BIN_OUTPUT="${BIN_NAME}"
#.${BIN_TYPE}"

#echo "${C_SOURCES[@]}"

#Compile
#Link
#Clear object files
arm-linux-gnueabihf-g++ -ggdb "$INCLUDE" "$LIB_INCLUDE" "${C_SOURCES[@]}" -o "${BIN_OUTPUT}"


#?UPDATE - SEND THE BINARY TO TARGET USING scp UTILITY 
TARGET_USER_NAME="pi"
TARGET_ADDRESS='192.168.0.139'
TARGET_FOLDER="/home/pi/share"
#"/home/pi/bin/"

SOURCE_FILE="${BIN_OUTPUT}"
CONFIG_FILE=("modbus.conf" "gc600.conf" "gc600.map")
scp -P 171 "$SOURCE_FILE" $TARGET_USER_NAME@$TARGET_ADDRESS:$TARGET_FOLDER \
&& \
scp -P 171 "${CONFIG_FILE[@]}" $TARGET_USER_NAME@$TARGET_ADDRESS:$TARGET_FOLDER

