# Field Data Logger Software
## Install
### Download the release version in [zip](https://github.com/niendicker/FieldDatalogger/archive/refs/tags/release.zip) or [tar.gz](https://github.com/niendicker/FieldDatalogger/archive/refs/tags/release.tar.gz) format. 

### From command line:
* **zip**
```
wget https://github.com/niendicker/FieldDatalogger/archive/refs/tags/release.zip
```
* **tar.gz**
```
wget https://github.com/niendicker/FieldDatalogger/archive/refs/tags/release.tar.gz
```
### Unpack file:
* **zip**
```
unzip release.zip
```
* **tar.gz**
```
tar -xf release.tar.gz
```

## Running
* After unpacking the release file, change the current directory to logger root directory (created at unpacking phase)
```
cd ./logger-release
```
And run the following commands:
* **x86**:
```
./bin/logger.bin ./devices/device.conf
``` 
```
./bin/logger_dbg.bin ./devices/device.conf
``` 

* **arm**:
```
./bin/logger.arm ./devices/device.conf
``` 
```
./bin/logger_dbg.arm ./devices/device.conf
``` 
Multiple instances can bin in parallel. Each instance can log data from a distinct device. 
> **Each device that will be monitored must have the configuration and map file.**

## Configuration

* The logger software sends [ModBus](https://modbus.org/) queries for each configured variable in the **mapFile**.
> Default value: *[./devices/device.mbr](./devices/device.mbr)*

* The interval between queries can be set on **pollingInterval_ms**
> Default value: 0 *no interval* 

* Iterations can be set on **pollingIterations**
> Default value: 0 *no limit*

* Maximum communication errors can be set on **pollingErrorMax**
> Default value: 0 *no limit*

* Timeout for a query reply can be set on **msTimeout**
> Default value: 1000ms. The value must be adjusted in function of connection latency. ***To low values can generate false communication errors***

### All above mentioned configurations can be found in the [default configuration file](./devices/device.conf).
