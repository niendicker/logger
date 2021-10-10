create schema if not exists modbuspoll;
CREATE TABLE modbuspoll.gc600_mains_modbuspoll ();
COMMENT ON TABLE modbuspoll.gc600_mains_modbuspoll IS 'One table for each device on each client site. Use add_column.sql to add device columns';
