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

## Configuration

* The logger software sends [ModBus](https://modbus.org/) queries for each configured variable in the **mapFile**.
> Default value: *[./run/devices/device.mbr](./run/devices/device.mbr)*

* The interval between queries can be set on **pollingInterval_ms**
> Default value: 0 *no interval* 

* Iterations can be set on **pollingIterations**
> Default value: 0 *no limit*

* Maximum communication errors can be set on **pollingErrorMax**
> Default value: 0 *no limit*

* Timeout for a query reply can be set on **msTimeout**
> Default value: 1000ms. The value must be adjusted in function of connection latency. ***To low values can generate false communication errors***

All above mentioned configurations can be found in the [default configuration file](./run/devices/device.conf).

## Running
* **x86**:
```
./run/logger.bin ./run/devices/device.conf
``` 
```
./run/logger_dbg.bin ./run/devices/device.conf
``` 

* **arm**:
```
./run/logger.arm ./run/devices/device.conf
``` 
```
./run/logger_dbg.arm ./run/devices/device.conf
``` 


Multiple instances can run in parallel. Each instance can log data from a distinct device. 
> **Each device that will be monitored must have the configuration and map file.**
