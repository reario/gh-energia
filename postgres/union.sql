-- select * from
-- ( 
-- (select min(ora) as o, kwh from energia where data='20130218' group by kwh order by o asc limit 1) as S1 UNION 
-- (select min(ora) as o, kwh from energia where data='20130218' group by kwh order by o desc limit 1 )

-- );

-- select to_char(min(S.data), 'month') as month, to_char(avg(S.Num),'999.99') as "media giornaliera" 
-- from (

--      select data,min(kWh),max(kWh),(max(kWh)-min(kWh)) as Num,to_char(avg(Num),'999.99') as "media giornaliera" 
--      from energia 
--      where data between '20130101' and date '20130101' + interval '1 month -1 day' 
--      group by data order by data
--      ) as S;

--with vals as (select data,max(kwh)-min(kwh) as kwh from energia where data between '20130201' and '20130228' group by data order by data) select avg(kwh) from vals;




-- select data,(max(kWh)-min(kWh)) as Num --sum(Num) as "media giornaliera"
--      from energia
--      where data between '20130201' and date '20130201' + interval '1 month -1 day'
--      group by data order by data;


select to_char(m.d,'TMMonth YYYY') as mese from (select date_trunc('month',data) as d from energia group by d order by d) as m;

select m.d as mese from (select date_trunc('month',data) as d from energia where data>'20120601' group by d order by d) as m;

select m.d from (select date_trunc('month',data) as d from energia group by d order by d) as m where m.s>'2012-04-01';

select m.d k.kwh from (select date_trunc('month',data) as d from energia group by d order by d) as m where m.s>'2012-04-01';

with vals as 
     (select data,max(kwh)-min(kwh) as kwh 
     from energia 
     where data between '20130201' and date '20130201' + interval '1 month -1 day' 
     group by data 
     order by data),

     mese as (select m.d from (select date_trunc('month',data) as d from energia group by d order by d) as m )

     select to_char(min(data),'TMMonth'),to_char(avg(kwh),'99.99') 
     from vals;





with vals as
     (select data,max(kwh)-min(kwh) as kwh
     from energia
     where data >=Me.m and data <=Me.m + interval '1 month -1 day'
     group by data
     order by data)  select data as d ,avg(kwh) as kwhm from vals as media 
     JOIN  mesi as Me ON media.dm=Me.m;


--  inner JOIN (select m.d as mese from (select date_trunc('month',data) as d from energia group by d order by d) as m) as kd ON kk.data=m.data;

select to_char(min(k.data), 'TMMonth') as mese,
       avg(k.kwh) as "media kWh" 
from (select data, max(kwh)-min(kwh) as kwh
     from energia 
     where data between and  + interval '1 month -1 day'
     group by data
     order by data) 
     as k 
     JOIN (select m.d as mese from (select date_trunc('month',data) as d from energia where data>'20120601' group by d order by d) as m) as f
     ON k.data=m.d;






select giornaliero.mese, avg(giornaliero.kwh)    
from 
(select e.data as mese,max(e.kwh)-min(e.kwh) as kwh
from energia as e, (select distinct date_trunc('month',data) as d from energia order by d) as m
where e.data between m.d and m.d + interval '1 month -1 day'
group by mese
order by mese) as giornaliero group by giornaliero.mese;



select k.data as mese, avg(k.kwh) as "media kWh"
from 
     (select distinct date_trunc('month',data) as d from energia) as m,
     (select data, max(kwh)-min(kwh) as kwh from energia 
     where data between m.d and m.d + interval '1 month -1 day' group by data order by data) as k
Where m.d=k.data group by mese;



-----------------------------------------------------------------------------------------------------------------
-- QUERY CHE CALCOLA LA MEDIA DI UN SINGOLO MESE
select to_char(min(k.data), 'TMMonth YYYY') as mese,
       to_char(avg(k.kwh),'99.99') as "media kWh" 
from (select data,max(kwh)-min(kwh) as kwh
     from energia
     where data between '2013-01-01 00:00:00+01' and date '20130101' + interval '1 month -1 day'
     group by data
     order by data) 
     as k;


-- QUERY FINALE CHE CALCOLA LA MEDIA DEI kWh consumati nei vari mesi ----
   select to_char(k.mese,'TMMonth YYYY') "mese di riferimento", sum(k.kwh) as "num kwh",to_char(avg(k.kwh),'99.99') as "media kWh"
   from 
     (select date_trunc('month',data) as mese, max(kwh)-min(kwh) as kwh from energia group by data) as k,
     (select distinct date_trunc('month',data) as d from energia where data >='20120501') as m -- log dei kwh parte da Maggio 2012
     
    Where k.mese = m.d
    Group by k.mese
    order by k.mese;

----- porting su mysql? Primo tentativo
   select k.mese as "mese di riferimento", sum(k.kwh) as "num kwh",avg(k.kwh) as "media kWh"
   from
     (select EXTRACT(MONTH FROM data) as mese, max(kwh)-min(kwh) as kwh from energia group by data) as k,
     (select distinct EXTRACT(MONTH FROM data) as d from energia where data >='20120501') as m -- log dei kwh parte da Maggio 2012

    Where k.mese = m.d
    Group by k.mese
    order by k.mese;




-- -- prima dell'indice sulla data
--  GroupAggregate  (cost=33321.86..33336.31 rows=200 width=12)
--    ->  Merge Join  (cost=33321.86..33330.62 rows=438 width=12)
--          Merge Cond: (k.mese = (date_trunc('month'::text, (public.energia.data)::timestamp with time zone)))
--          ->  Sort  (cost=16661.48..16662.57 rows=438 width=12)
--                Sort Key: k.mese
--                ->  Subquery Scan on k  (cost=16630.22..16642.26 rows=438 width=12)
--                      ->  HashAggregate  (cost=16630.22..16637.88 rows=438 width=8)
--                            ->  Seq Scan on energia  (cost=0.00..12181.98 rows=593098 width=8)
--          ->  Sort  (cost=16660.38..16661.48 rows=438 width=8)
--                Sort Key: (date_trunc('month'::text, (public.energia.data)::timestamp with time zone))
--                ->  HashAggregate  (cost=16630.22..16636.78 rows=438 width=4)
--                      ->  Seq Scan on energia  (cost=0.00..15147.47 rows=593098 width=4)

-- -- dopo l'indice sulla data
--  GroupAggregate  (cost=33221.06..33235.51 rows=200 width=12)
--    ->  Merge Join  (cost=33221.06..33229.82 rows=438 width=12)
--          Merge Cond: (k.mese = (date_trunc('month'::text, (public.energia.data)::timestamp with time zone)))
--          ->  Sort  (cost=16611.08..16612.17 rows=438 width=12)
--                Sort Key: k.mese
--                ->  Subquery Scan on k  (cost=16579.81..16591.86 rows=438 width=12)
--                      ->  HashAggregate  (cost=16579.81..16587.48 rows=438 width=8)
--                            ->  Seq Scan on energia  (cost=0.00..12153.18 rows=590218 width=8)
--          ->  Sort  (cost=16609.98..16611.08 rows=438 width=8)
--                Sort Key: (date_trunc('month'::text, (public.energia.data)::timestamp with time zone))
--                ->  HashAggregate  (cost=16579.81..16586.38 rows=438 width=4)
--                      ->  Seq Scan on energia  (cost=0.00..15104.27 rows=590218 width=4)


-- -------------------------------------------------------------------------------------------------------------------------------
-- Trova il min e il max valore di Volt in una giornata

select data,ora,volt from energia where volt=(select max(volt) from energia where data between '20130201' and '20130228') 
       		     	     	   and 
       		     	     	   data between '20130201' and '20130228'
UNION ALL 
select data,ora,volt from energia where volt=(select min(volt) from energia where data between '20130201' and '20130228')
       		     	     	   and 
				   data between '20130201' and '20130228';

-- indica anche nella colonna 'dummy' se il valore ritornato e' di tipo min o max. 
-- Utile se devo fare la query che si estende ad un range temporale di piu' giorni 

select data,ora,volt,'MAX' as o from energia where volt=(select max(volt) from energia where data=CURRENT_DATE) and data=CURRENT_DATE
UNION ALL
select data,ora,volt,'MIN' as o from energia where volt=(select min(volt) from energia where data=CURRENT_DATE) and data=CURRENT_DATE;

select ora,volt from energia where data=CURRENT_DATE AND (
       (volt=(select min(volt) from energia where data=CURRENT_DATE)) OR 
       (volt=(select max(volt) from energia where data=CURRENT_DATE))
);



-- -----------------------------------TROVA MIN MAX PER OGNI GIORNO DI UN DATO MESE ----------------------------
select data, ora, value, minmax from (
(Select e.data as data, e.ora as ora, t.min as value, 'MIN' as minmax
from
   (select data, min(volt) as min
    from energia
   group by data) as t, energia as e
where e.data=t.data and e.volt=t.min)
union all
(Select e.data as data, e.ora as ora, t.max as value, 'MAX' as minmax
from
   (select data, max(volt) as max
    from energia
   group by data) as t, energia as e
where
   e.data=t.data
and e.volt=t.max) ) as foo where data between '20130101' and '20130228' order by data;

-- --------------------------------------------------------------------------------------------------------------


-- prende solo i MINIMI
select data, ora, value, minmax from
(Select e.data as data, e.ora as ora, t.min as value, 'MIN' as minmax
from
   (select data, min(volt) as min
    from energia
   group by data) as t, energia as e
where e.data=t.data and e.volt=t.min) as foo where data between '20130201' and '20130228' order by data;
-- --------------------------------------------------------------------------------------------------------------

select avg(volt) from energia where data between 20130102 and 20130228;





