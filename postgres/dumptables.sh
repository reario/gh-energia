#!/bin/sh
now=`date +"%d-%m-%Y"`

# dump i contenuti
/usr/bin/psql -U reario -d reario_db -c "copy energia to stdout csv header;" | gzip -v9 - > /home/reario/energia/dump/energia.$now.csv.gz
/usr/bin/psql -U reario -d reario_db -c "copy events to stdout csv header;" | gzip -v9 - > /home/reario/energia/dump/events.$now.csv.gz

# dump lo schema di view e tabelle
/usr/bin/pg_dump -s -t public.energia -s -t public.events -s -t public.event_distribution -s -t public.maxdailyon -U reario -d reario_db | gzip -v9 - > /home/reario/energia/dump/views.$now.gz

# rimuovi i file + vecchi di 2 giorni
/usr/bin/find /home/reario/energia/dump -mindepth 1 -mtime +2 -delete
