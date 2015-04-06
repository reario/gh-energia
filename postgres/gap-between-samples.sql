-- trova i gap temporali>30 minuti nei campionamenti
select data,prev::text::interval::time,ora,deltats::text::interval::time as timegap from

(select data,ora,dummy,ts, ts - lag (ts) over (order by ts) as deltats, lag(ts,1) over (order by ts) as prev from

(select data,ora,dummy,EXTRACT(EPOCH FROM (data + ora)) as ts from energia ) as vittorio) as g 
where deltats::text::interval >= '00:30:00' order by timegap
-- where ts=1345726358 or ts=1345810209
-- where data>='20120823' limit 759
