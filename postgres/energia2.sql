--drop table energia if not exist;
CREATE TABLE energia (
data date,
ora time,
volt float,
ampere float,
kW float,
kWPT float,
kWPOZZO float,
dummy SERIAL,
kWh int,
CONSTRAINT data_ora_energia PRIMARY KEY (data,ora)
);

CREATE TABLE ss (
data date,
ora time,
secA integer,
secP integer,
scaA integer,
scaP integer,
CONSTRAINT data_ora_ss PRIMARY KEY (data,ora)
);


-- --drop table scatti if not exist;
-- CREATE TABLE scatti (
-- dummy serial,
-- data date,
-- ora time,
-- autoc integer,
-- pozzo integer,
-- CONSTRAINT data_ora_scatti PRIMARY KEY (data,ora)
-- );

-- --drop table scatti if not exist;
-- CREATE TABLE secondi (
-- dummy serial,
-- data date,
-- ora time,
-- autoc integer,
-- pozzo integer,
-- CONSTRAINT data_ora_secondi PRIMARY KEY (data,ora)
-- );

-- --drop view ss;
-- Create VIEW ss AS
--        select
--        secondi.data AS data,
--        secondi.ora AS ora,
--        secondi.autoc AS secondi_autoclave,
--        scatti.autoc AS scatti_autoclave,
--        secondi.pozzo AS second_pozzo,
--        scatti.pozzo AS scatti_pozzo
--        FROM secondi,scatti WHERE (secondi.data = scatti.data) AND (secondi.ora = scatti.ora);



-- Create VIEW ss AS 
       -- select 
       -- secondi.data AS data,
       -- secondi.ora AS ora,
       -- secondi.autoc AS secondi_autoclave,
       -- scatti.autoc AS scatti_autoclave,
       -- secondi.pozzo AS second_pozzo,
       -- scatti.pozzo AS scatti_pozzo 
       -- from (secondi join scatti) 
       -- where (secondi.id = scatti.id);