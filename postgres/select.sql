#!/bin/bash



#-- mostra statistiche scatti e secondi raggruppate per giorno
ss="select * from (select ss.data,sum(ss.scaa) as ScattiAutoclave, sum(ss.scap) as ScattiPozzo, sum(ss.seca) as SecondiAutoclave, sum(ss.secp) as secondiPozzo from ss where (data>='20130101') group by data) as sub order by data;"


for arg in "$@"

do

#echo "$arg"

#-- mostra min e max kWh raggruppato per giorno
kWh="select * from (select data, max(kWh)-min(kWh) as \"Consumi(kWh)\" from energia where data='$arg' group by data) as konsumi order by data;"
kWh1="(select min(ora) as o, kwh as kwh1 from energia where data='$arg' group by kwh1 order by o asc limit 1)
 UNION
(select min(ora) as o, kwh as kwh2 from energia where data='$arg' group by kwh2 order by o desc limit 1);"


ss="select * from (select ss.data,sum(ss.scaa) as ScaAut, sum(ss.scap) as ScaPoz, sum(ss.seca) as SecAut, sum(ss.secp) as secPoz from ss where data='$arg' group by data) as sub order by data;"
ssav="select * from (select ss.data,sum(ss.scaa) as ScattiAutoclave, avg(ss.scaa) as average, sum(ss.scap) as ScattiPozzo,avg(ss.scap) as average, sum(ss.seca) as SecondiAutoclave, avg(ss.seca) as average, sum(ss.secp) as secondiPozzo from ss where data='$arg' group by data) as sub order by data;"

psql -U reario reario_db -c "$kWh"
psql -U reario reario_db -c "$kWh1"
psql -U reario reario_db -c "$ss"
psql -U reario reario_db -c "$ssav"

done



#-- mostra statistiche scatti e secondi raggruppate per giorno
#ss="select * from (select ss.data,sum(ss.scaa) as ScattiAutoclave, sum(ss.scap) as ScattiPozzo, sum(ss.seca) as SecondiAutoclave, sum(ss.secp) as secondiPozzo from ss where (data>='20130101') group by data) as sub order by data;"

#-- mostra min e max kWh raggruppato per giorno
#kWh="select * from (select data, max(kWh)-min(kWh) as \"Consumi(kWh)\" from energia where data>='20130201' AND data<='20130228' group by data) as konsumi order by data;"



