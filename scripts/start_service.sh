#! /bin/bash

binDir="$(pwd)/bin"
configFile="${1}"

systemdUnitsDir="/lib/systemd/system"
templateUnitFile="logger@default.service"
{
  echo "[Unit]";
  echo "Description=Configuration ${1}";
  echo "After=multi-user.target";
  echo " ";
  echo "[Service]";
  echo "Type=simple";
  echo "User=pi";
  echo "WorkingDirectory=${binDir}";
  echo "ExecStart=${binDir}/logger.arm ${configFile}";
  echo " ";
  echo "RestartSec=10";
  echo "Restart=on-failure";
  echo " ";
  echo "[Install]";
  echo "WantedBy=multi-user.target";
} >> $systemdUnitsDir/$templateUnitFile
#mv ./$templateUnitFile $systemdUnitsDir

sudo ln -s $systemdUnitsDir/logger@default.service $systemdUnitsDir

sudo systemctl enable $templateUnitFile && \
sudo systemctl start  $templateUnitFile 
sudo systemctl status $templateUnitFile
