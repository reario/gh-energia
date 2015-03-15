-- \set input 'INTERNET'
-- \set data 2014-06-01
-- \set orastart 23:00:56
-- \set orastop 23:01:21
-- \set ts 2311

-- -- EVENTO ON
-- update EVENTKEYS set ts=:'ts' where input=:'input';
-- insert into events (data,input,orastart,orastop,ts) VALUES (:'data',:'input',:'orastart',NULL,:'ts');

-- -- EVENTO OFF
-- update events set orastop=:'orastop',ts=(select ts from eventkeys where input='COMPLETATO') 
--        where ts=(select ts from eventkeys where input=:'input');
--select completati.input,sum(elapsed) from (select data,input,orastart,orastop,orastop-orastart as elapsed from events where events.ts=0) as completati group by input;




drop table IF EXISTS events;
--drop table IF EXISTS eventkeys;
drop type inputs;
drop type outputs;

CREATE TYPE inputs AS ENUM ( 
 'COMPLETATO',
 'AUTOCLAVE', -- 0
 'POMPA_SOMMERSA', -- 1
 'RIEMPIMENTO', -- 2
 'LUCI_ESTERNE_SOTTO', -- 3
 'CENTR_R8', --4
 'LUCI_GARAGE_DA_4', --5
 'LUCI_GARAGE_DA_2', --6
 'LUCI_TAVERNA_1_di_2', --7
 'LUCI_TAVERNA_2_di_2', --8
 'INTERNET', --9
 'C9912', --10
 'LUCI_CUN_LUN', --11
 'LUCI_CUN_COR', --12
 'LUCI_STUDIO_SOTTO', --13
 'LUCI_ANDRONE_SCALE', --14
 'GENERALE_AUTOCLAVE',--15
 'LUCI_CANTINETTA' --16
);

CREATE TYPE outputs AS ENUM ( 
 '__AUTOCLAVE' -- 0
 '__POMPA_SOMMERSA', -- 1
 '__RIEMPIMENTO', -- 2
 '__LUCI_ESTERNE_SOTTO', -- 3
 '__CENTR_R8', --4
 '__LUCI_GARAGE_DA_4', --5
 '__LUCI_GARAGE_DA_2', --6
 '__LUCI_TAVERNA_1_di_2', --7
 '__LUCI_TAVERNA_2_di_2', --8
 '__INTERNET' --9
 '__C9912', --10
 '__LUCI_CUN_LUN', --11
 '__LUCI_CUN_COR', --12
 '__LUCI_STUDIO_SOTTO' --13
 '__LUCI_ANDRONE_SCALE', --14
 '__GENERALE_AUTOCLAVE',--15
 '__LUCI_CANTINETTA' --16
);


--CREATE TABLE eventkeys (
--input inputs,
--ts bigint PRIMARY KEY
--);


CREATE TABLE events (
data date,
input inputs, -- forse era meglio chiamarlo NOME
orastart time,
orastop time,
ts bigint --references eventkeys(ts)
);

-- insert into eventkeys (input,ts) VALUES ('COMPLETATO',0);
-- insert into eventkeys (input,ts) VALUES ('AUTOCLAVE',100);
-- insert into eventkeys (input,ts) VALUES ('POMPA_SOMMERSA',101);
-- insert into eventkeys (input,ts) VALUES ('RIEMPIMENTO',102);
-- insert into eventkeys (input,ts) VALUES ('LUCI_ESTERNE_SOTTO',103);
-- insert into eventkeys (input,ts) VALUES ('CENTR_R8',104);
-- insert into eventkeys (input,ts) VALUES ('LUCI_GARAGE_DA_4',105);
-- insert into eventkeys (input,ts) VALUES ('LUCI_GARAGE_DA_2',106);
-- insert into eventkeys (input,ts) VALUES ('LUCI_TAVERNA_1_di_2',107);
-- insert into eventkeys (input,ts) VALUES ('LUCI_TAVERNA_2_di_2',108);
-- insert into eventkeys (input,ts) VALUES ('INTERNET',109);
-- insert into eventkeys (input,ts) VALUES ('C9912',110);
-- insert into eventkeys (input,ts) VALUES ('LUCI_CUN_LUN',111);
-- insert into eventkeys (input,ts) VALUES ('LUCI_CUN_COR',112);
-- insert into eventkeys (input,ts) VALUES ('LUCI_STUDIO_SOTTO',113);
-- insert into eventkeys (input,ts) VALUES ('LUCI_ANDRONE_SCALE',114);
-- insert into eventkeys (input,ts) VALUES ('GENERALE_AUTOCLAVE',115);
-- insert into eventkeys (input,ts) VALUES ('LUCI_CANTINETTA',116);
