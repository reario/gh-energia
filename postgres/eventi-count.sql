
--CREATE VIEW event_distribution as (
select to_char(cartesian.ora::timestamp, 'HH24:MI:SS') as "Time range", 
--ora,
 
 sum(cartesian.AUTOCLAVE) AS AUTOCLAVE,
 sum(cartesian.POMPA_SOMMERSA) AS "Pompa Sommersa",
 sum(cartesian.LUCI_STUDIO_SOTTO) AS "Studiio",
 sum(cartesian.RIEMPIMENTO) AS Riempimento,
 sum(cartesian.LUCI_CANTINETTA) AS "Luci Cantinetta",
 sum(cartesian.LUCI_GARAGE_DA_4) AS "garage da 4",
 sum(cartesian.CENTR_R8) AS "CENTR_R8",
-- sum(cartesian.INTERNET) AS "INTERNET",
-- sum(cartesian.GENERALE_AUTOCLAVE) AS "GEN_AUTOCLAVE",
 sum(cartesian.LUCI_ANDRONE_SCALE) AS "ANDRONE_SCALE",
 sum(cartesian.LUCI_GARAGE_DA_2) AS "GARAGE_DA_2",
 sum(cartesian.LUCI_CUN_LUN) AS "CUN_LUN",
 sum(cartesian.LUCI_TAVERNA_1_di_2) AS "TAVERNA_1_di_2",
 sum(cartesian.LUCI_ESTERNE_SOTTO) AS "ESTERNE_SOTTO",
 sum(cartesian.LUCI_TAVERNA_2_di_2) AS "TAVERNA_2_di_2"

  
 from (select ora,pinco.elaps,
            case when (input='AUTOCLAVE' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as AUTOCLAVE,
	    case when (input='POMPA_SOMMERSA' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as POMPA_SOMMERSA,
      	    case when (input='LUCI_STUDIO_SOTTO' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_STUDIO_SOTTO,
      	    case when (input='RIEMPIMENTO' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as RIEMPIMENTO,
      	    case when (input='LUCI_CANTINETTA' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_CANTINETTA,
      	    case when (input='LUCI_GARAGE_DA_4' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_GARAGE_DA_4,
      	    case when (input='CENTR_R8' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as CENTR_R8,
      	    case when (input='INTERNET' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as INTERNET,
      	    case when (input='GENERALE_AUTOCLAVE' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour')
	    then 1 ELSE 0 end as GENERALE_AUTOCLAVE,
      	    case when (input='LUCI_ANDRONE_SCALE' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_ANDRONE_SCALE,
      	    case when (input='LUCI_GARAGE_DA_2' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour')
	    then 1 ELSE 0 end as LUCI_GARAGE_DA_2,
	    case when (input='LUCI_CUN_LUN' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_CUN_LUN,
	    case when (input='LUCI_TAVERNA_1_di_2' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_TAVERNA_1_di_2,
	    case when (input='LUCI_TAVERNA_2_di_2' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_ESTERNE_SOTTO,
	    case when (input='LUCI_TAVERNA_2_di_2' and (current_date + orastart)>=pallo.ora and (current_date + orastart)<ora+interval '1 hour') 
	    then 1 ELSE 0 end as LUCI_TAVERNA_2_di_2
--	 from (select input,(current_date + orastart) from events) as pinco, 

	 from (select input,orastart,orastop-orastart as elaps from events ) as pinco, 
	      (select ore.t as ora from generate_series(current_date,current_date + 1,interval '1 hour') as ore(t)) as pallo) as cartesian 
	      group by cartesian.ora order by ora


--)

-- from (select input,(current_date + orastart) from events where data=current_date) 
-- as pinco, (select * from serieore as ora) as pallo) as cartesian group by ora order by ora;
