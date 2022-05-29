
--- Field Data Logger Software ---

The modbusPoll software sends a ModBus query for each configured variable in the [mapFile] at specified interval [pollingInterval_ms=0]. 0 = no interval. 
Iterations can be set on [pollingIterations=0]. 0 = no limit.
Maximum communication errors can be set on [pollingErrorMax=0] 0 = no limit.
Timeout for a query reply can be set on [msTimeout=1000]. 
 *The value must be adjusted in function of connection latency. To low values can generate false communication errors.

All above mentioned configurations can be found in the default configuration file: 
[/devices/device.conf].

Default mapFile:
[/devices/device.mbr]

Running a instance:
./modbusPoll.bin /path/to/configuration/file

Ex: 
cd [/path/to/modbusPoll/root/folder]
./modbusPoll.bin ./devices/device.conf

* Multiple instances can run in parallel.

