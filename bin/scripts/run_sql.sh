#!/bin/bash

user="modbuspoll"
modbusDatabase=$user

PGPASSWORD='n13nd1ck3r' \
psql -U $user -d $modbusDatabase -f "'$1'" 