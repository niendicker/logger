ALTER TABLE public.gc600_mains_modbuspoll ADD local_timestamp timestamptz NOT NULL,
  ADD Generator_Frequency varchar(20) NOT NULL,
  ADD Generator_Avg_Voltage varchar(20) NOT NULL,
  ADD Generator_Apparent_Power varchar(20) NOT NULL,
  ADD Generator_Avg_Active_Power varchar(20) NOT NULL,
  ADD Generator_Avg_Reactive_Power varchar(20) NOT NULL,
  ADD Mains_Frequency varchar(20) NOT NULL,
  ADD Mains_Avg_Voltage varchar(20) NOT NULL,
  ADD Mains_Active_Energy varchar(20) NOT NULL;