/usr/lib/postgresql/9.3/bin/pg_ctl -D /var/lib/postgresql/9.3/main/ -o '--config-file=/etc/postgresql/9.3/main/postgresql.conf' -l file_log start/stop
/usr/lib/postgresql/10/main/bin/pg_ctl -D /var/lib/postgresql/10/main/ -o '--config-file=/etc/postgresql/10/main/postgresql.conf' -l file_log start/stop

## dump di tabelle da un DB all'altro sulla stessa macchina che girano su porte differenti
/usr/lib/postgresql/9.3/bin/pg_dump -h localhost -p 5432 -d reario_db -t public.event_distribution | /usr/lib/postgresql/10/bin/psql -h localhost -p 5433 -U reario -d reario_db -f -

