#!/bin/bash

user="modbuspoll"
modbusDatabase=$user
modbusRelation="mb_reply_delay"

PGPASSWORD='n13nd1ck3r' \
psql \
-q \
-U $user \
-d $modbusDatabase \
-c "INSERT INTO public.$modbusRelation VALUES ('$1',$2, now(), 'ms' );" 