--\H
select input,sum(tempo.elaps) as "tempo totale",count(*) as "num accenzioni",data as "giorno" from (select *,orastop-orastart as elaps from events where data=current_date order by orastart) as tempo group by input,data;
