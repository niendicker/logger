#! /bin/bash
# $1: How much instances will run
#     Max = 50
# $2: Version to run 
#     0 = Run production version anything else = Run debug version  
configDir="./dev/"

if [ "$1" -ne 0 ];  
then #RUN DEBUG VERSION
  echo "Running in debug mode..."
  cd .. &&               \
  ./modbusPoll_dbg.arm "${configDir}"
else #RUN PRODUCTION VERSION
  echo "Running in production mode..."
  cd .. &&          \
  nohup             \
  ./modbusPoll.arm "${configDir}" \
  &>/dev/null & disown          
fi