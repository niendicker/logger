#!/bin/bash

#? BUILD TEST UNIT
INCLUDE="-I../src"
LIB_INCLUDE="-L/usr/local/lib"

#MAIN_SOURCES_HOME="../src"

TEST_SOURCES_HOME="../test"
C_SOURCES=( "${TEST_SOURCES_HOME}/http_get.c")

BIN_HOME="${TEST_SOURCES_HOME}/bin"
BIN_SUFIX=$(date '+%d%m%Y%H%M')
BIN_OUTPUT="${BIN_HOME}/UT_${BIN_SUFIX}"

#Compile and Link for x86 architecture
gcc "$INCLUDE" "$LIB_INCLUDE" "${C_SOURCES[@]}" -o "${BIN_OUTPUT}"

