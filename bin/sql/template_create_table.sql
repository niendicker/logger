CREATE TABLE modbuspoll.deviceid (
	local_timestamp timestamptz NOT NULL DEFAULT '1988-04-03 00:00:01-03',
	data1 varchar(20) NOT NULL DEFAULT 'data',
	datan varchar(20) NOT NULL DEFAULT 'data',
  metrics varchar(10) NOT NULL DEFAULT 'metrics'
);
COMMENT ON TABLE modbuspoll.deviceid IS 'One table for each device on each client site';

-- Column comments

COMMENT ON COLUMN modbuspoll.deviceid.local_timestamp IS 'Very special day';
COMMENT ON COLUMN modbuspoll.deviceid.data1 IS 'First data readded from device';
COMMENT ON COLUMN modbuspoll.deviceid.datan IS 'Last data readded from device';
COMMENT ON COLUMN modbuspoll.deviceid.metrics IS 'Data measurement unit';
