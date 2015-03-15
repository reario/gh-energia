select st.ora,stop.ora 
from
(select ora from input_events where (input='AUTOCLAVE' AND state='ON') order by ora) as st,
(select ora from input_events where (input='AUTOCLAVE' AND state='OFF') order by ora) as stop where group by st.ora;


select start.ora as on,stop.ora as off
from
(select ora from input_events where (input='AUTOCLAVE' AND state='ON') group by ora order by ora ) as start,
(select ora from input_events where (input='AUTOCLAVE' AND state='OFF') group by ora order by ora ) as stop;

select ora from input_events where (input='AUTOCLAVE' AND state='ON') order by ora UNION select ora from input_events where (input='AUTOCLAVE' AND state='OFF' order by ora)


Create VIEW AUTOCLAVE_ON AS select data,ora from input_events WHERE input='AUTOCLAVE' AND state='ON' order by data,ora;
Create VIEW AUTOCLAVE_OFF AS select data,ora from input_events WHERE input='AUTOCLAVE' AND state='OFF' order by data,ora;
