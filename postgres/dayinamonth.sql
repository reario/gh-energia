CREATE FUNCTION num_days(int, int) RETURNS float8 AS -- years, months
       'SELECT date_part(''day'',
       	       (($1::text || ''-'' || $2::text || ''-01'')::date
	       		     + ''1 month''::interval
			       	   - ''1 day''::interval)) AS days' LANGUAGE 'sql';