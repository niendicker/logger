#!/bin/bash

superuser="postgres"
user="modbuspoll"
password="n13nd1ck3r"
database=$user
modbusRelation="mb_reply_delay"

echo "#----------------------------------------------------------"
echo "# CREATE DEFAULT USER"
echo "#----------------------------------------------------------"
sudo -u $superuser psql -c "CREATE USER $user WITH \
CONNECTION LIMIT 10 LOGIN ENCRYPTED PASSWORD '$password' ; " && \
echo "User $user created "
echo "#----------------------------------------------------------" 
echo "# CREATE DEFAULT DATABASE" 
echo "#----------------------------------------------------------" 
sudo -u $superuser psql -c "CREATE DATABASE $database WITH \
OWNER=$user ENCODING=UTF_8 ;" && \
echo "Database $database created "

echo "#----------------------------------------------------------"
echo "# CREATE DEFAULT RELATION" 
echo "#----------------------------------------------------------" 
sudo -u $superuser psql -d $database -c "CREATE TABLE \
$database.public.$modbusRelation ( \
deviceid varchar(50) NOT NULL DEFAULT 'Invalid', \
delay    real        NOT NULL DEFAULT 0.0, \
time     timestamptz NOT NULL, \
metrics  varchar(2)  NOT NULL DEFAULT 'ms' ); \
-- Column comments COMMENT ON COLUMN public.$modbusRelation.deviceid IS \
'Device ID need to be unique for each device on local network';" && \
echo "Relation $modbusRelation created "

echo "#----------------------------------------------------------"
echo "# GRANT ACCESS OF USER TO RELATION " 
echo "#----------------------------------------------------------" 
sudo -u $superuser psql -d $database -c "GRANT ALL PRIVILEGES ON DATABASE $database TO $user; "
