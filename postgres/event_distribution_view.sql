
SELECT to_char((cartesian.ora)::timestamp without time zone,'HH24:MI:SS'::text) AS "Time range",
    sum(cartesian.autoclave) AS autoclave,
    sum(cartesian.pompa_sommersa) AS "Pompa Sommersa",
    sum(cartesian.luci_studio_sotto) AS "Studiio",
    sum(cartesian.riempimento) AS riempimento,
    sum(cartesian.luci_cantinetta) AS "Luci Cantinetta",
    sum(cartesian.luci_garage_da_4) AS "garage da 4",
    sum(cartesian.centr_r8) AS "CENTR_R8",
    sum(cartesian.luci_androne_scale) AS "ANDRONE_SCALE",
    sum(cartesian.luci_garage_da_2) AS "GARAGE_DA_2",
    sum(cartesian.luci_cun_lun) AS "CUN_LUN",
    sum(cartesian.luci_taverna_1_di_2) AS "TAVERNA_1_di_2",
    sum(cartesian.luci_esterne_sotto) AS "ESTERNE_SOTTO",
    sum(cartesian.luci_taverna_2_di_2) AS "TAVERNA_2_di_2"
FROM (
    SELECT pallo.ora,
    	CASE
        WHEN (((pinco.input = 'AUTOCLAVE'::inputs) AND
	((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND 
	((('now'::text)::date + pinco.orastart) < (pallo.ora + 
	'01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS autoclave,
        
	CASE
        WHEN (((pinco.input = 'POMPA_SOMMERSA'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS pompa_sommersa,
        
        CASE
	WHEN (((pinco.input = 'LUCI_STUDIO_SOTTO'::inputs) AND 
	((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND 
	((('now'::text)::date + pinco.orastart) < (pallo.ora + 
	'01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_studio_sotto,
        
	CASE
        WHEN (((pinco.input = 'RIEMPIMENTO'::inputs) AND 
	((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND 
	((('now'::text)::date + pinco.orastart) < (pallo.ora + '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS riempimento,
        
	CASE
        WHEN (((pinco.input = 'LUCI_CANTINETTA'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_cantinetta,
        
	CASE
        WHEN (((pinco.input = 'LUCI_GARAGE_DA_4'::inputs) AND 
	((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND 
	((('now'::text)::date + pinco.orastart) < (pallo.ora + '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_garage_da_4,
        
	CASE
        WHEN (((pinco.input = 'CENTR_R8'::inputs) AND 
	((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND 
	((('now'::text)::date + pinco.orastart) < (pallo.ora + 
	'01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS centr_r8,
        
	CASE
        WHEN (((pinco.input = 'INTERNET'::inputs) AND ((('now'::text)::date +
        pinco.orastart) >= pallo.ora)) AND ((('now'::text)::date +
        pinco.orastart) < (pallo.ora + '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS internet,
        
	CASE
        WHEN (((pinco.input = 'GENERALE_AUTOCLAVE'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) THEN 1
        ELSE 0
        END AS generale_autoclave,
        
	CASE
        WHEN (((pinco.input = 'LUCI_ANDRONE_SCALE'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_androne_scale,
        
	CASE
        WHEN (((pinco.input = 'LUCI_GARAGE_DA_2'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_garage_da_2,
        
        CASE
        WHEN (((pinco.input = 'LUCI_CUN_LUN'::inputs) AND 
	((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND 
	((('now'::text)::date + pinco.orastart) < (pallo.ora + 
	'01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_cun_lun,
        
	CASE
        WHEN (((pinco.input = 'LUCI_TAVERNA_1_di_2'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) THEN 1
                    ELSE 0
        END AS luci_taverna_1_di_2,
                CASE
        WHEN (((pinco.input = 'LUCI_TAVERNA_2_di_2'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_taverna_2_di_2,
        
	CASE
        WHEN (((pinco.input = 'LUCI_ESTERNE_SOTTO'::inputs) AND
        ((('now'::text)::date + pinco.orastart) >= pallo.ora)) AND
        ((('now'::text)::date + pinco.orastart) < (pallo.ora +
        '01:00:00'::interval))) 
	THEN 1
        ELSE 0
        END AS luci_esterne_sotto
    
    FROM (
        SELECT events.input, events.orastart
        FROM events
        ) pinco, 
	(SELECT ore.t AS ora
        FROM generate_series((('now'::text)::date)::timestamp with time zone,
            ((('now'::text)::date + 1))::timestamp with time zone,'01:00:00'::interval) ore(t)
        ) pallo
    ) cartesian
GROUP BY cartesian.ora
ORDER BY cartesian.ora;
