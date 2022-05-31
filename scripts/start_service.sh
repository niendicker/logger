#! /bin/bash
#
# Must be executed as super user from project root dir
#
# Generate service template file for systemctl
# Enable the service and start it with default configuration 
#

rootdir=$(pwd)
bindir="${rootdir}/bin"
confdir="${rootdir}/devices"
configFile="${confdir}/device.conf"
execName="logger"
execFile="${bindir}/${execName}.arm"

user="logger"
systemdUnitsDir="/lib/systemd/system"

templateUnitFile="${execName}@.service"
{
  echo "[Unit]";
  echo "Description=Using configuration: ${configFile}";
  echo "After=multi-user.target";
  echo " ";
  echo "[Service]";
  echo "Type=simple";
  echo "User=${user}";
  echo "WorkingDirectory=${bindir}";
  echo "ExecStart=${execFile} ${configFile}";
  echo " ";
  echo "RestartSec=10";
  echo "Restart=on-failure";
  echo " ";
  echo "[Install]";
  echo "WantedBy=multi-user.target";
} >> $systemdUnitsDir/$templateUnitFile

defaultService="${execName}@default.service"
sudo ln -s $systemdUnitsDir/$defaultService $systemdUnitsDir

sudo systemctl enable $defaultService && \
sudo systemctl start  $defaultService 
sudo systemctl status $defaultService
