#! /bin/bash

mbpollHome="/home/pi/run/bin"
configFile="./dev/$1"

systemdUnitsDir="/lib/systemd/system"
templateUnitFile="modbuspoll@.service"
{
  echo "[Unit]";
  echo "Description=Configuration $1";
  echo "After=multi-user.target";
  echo " ";
  echo "[Service]";
  echo "Type=simple";
  echo "User=pi";
  echo "WorkingDirectory=${mbpollHome}";
  echo "ExecStart=${mbpollHome}/modbusPoll.arm ${configFile}";
  echo " ";
  echo "RestartSec=10";
  echo "Restart=on-failure";
  echo " ";
  echo "[Install]";
  echo "WantedBy=multi-user.target";
} >> $systemdUnitsDir/$templateUnitFile
#mv ./$templateUnitFile $systemdUnitsDir

ln -s $systemdUnitsDir/modbuspoll@gc600.service $systemdUnitsDir

systemctl enable $templateUnitFile
systemctl start  $templateUnitFile
systemctl status $templateUnitFile
