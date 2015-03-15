--\H
select input,sum(tempo.elaps),count(*),data as accenzioni from (select *,orastop-orastart as elaps from events where data=current_date order by orastart) as tempo group by input,data;
