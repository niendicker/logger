#! /bin/bash
# $1: How much instances will run
#     Max = 50
# $2: Version to run 
#     0 = Run production version anything else = Run debug version  

configFile="./dev/gc600.conf"
pollingInterval_ms=100
pollingIteractions=0 
pollingErrorMax=100
maxInstances=50

if [ $# -lt 2 ]; 
then
  echo "Provide args"
  exit
fi

if [ "$1" -gt $maxInstances ]; then
  echo "Can't run $1 instances. Max $maxInstances"
  exit
fi

for instances in $(seq 1 "$1"); 
do

if [ "$2" -ne 0 ];  
then #RUN DEBUG VERSION
  echo "Loading instance $instances in debug mode..."
  cd ../bin &&            \
  ./modbusPoll_dbg.arm    \
  "${configFile}"         \
  "${pollingInterval_ms}" \
  "${pollingIteractions}" \
  "${pollingErrorMax}"    \
  &                       \
  disown
else #RUN PRODUCTION VERSION
  echo "Loading instance $instances in production mode..."
  cd ../bin &&            \
  nohup                   \
  ./modbusPoll.arm        \
  "${configFile}"         \
  "${pollingInterval_ms}" \
  "${pollingIteractions}" \
  "${pollingErrorMax}"    \
  &>/dev/null             \
  &                       
  disown          

fi

done