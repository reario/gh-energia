create or replace view event_distribution as (
select * from crosstab('
select
width_bucket(extract(hour from events.orastart) ,1,24,24) as buckets
,input
,count(*)::integer as avvii
from events 
left join (select input as i from events group by "input") as _list on _list.i=events."input" 
group by buckets,input
order by buckets,input
',
'select "input" from events group by "input" order by "input"'
) as final_result(
bouckets integer,
"AUTOCLAVE" integer,
"C9912" integer,
"CENTR_R8" integer,
"FARI_ESTERNI" integer,
"FARI_ESTERNI_SOTTO" integer,
"GENERALE_AUTOCLAVE" integer,
"INTERNET" integer,
"LUCI_ANDRONE_SCALE" integer,
"LUCI_CANTINETTA" integer,
"LUCI_CUN_COR" integer,
"LUCI_CUN_LUN" integer,
"LUCI_ESTERNE_SOTTO" integer,
"LUCI_GARAGE_DA_2" integer,
"LUCI_GARAGE_DA_4" integer,
"LUCI_STUDIO_SOTTO" integer,
"LUCI_TAVERNA_1_di_2" integer,
"LUCI_TAVERNA_2_di_2" integer,
"OTB_DIN_10" integer,
"PLC_INPUT" integer,
"PLC_INPUT_17" integer,
"PLC_INPUT_18" integer,
"PLC_INPUT_19" integer,
"PLC_INPUT_20" integer,
"PLC_INPUT_21" integer,
"PLC_INPUT_22" integer,
"PLC_TM2_INPUT_0" integer,
"POMPA_SOMMERSA" integer,
"RIEMPIMENTO" integer
)
)
