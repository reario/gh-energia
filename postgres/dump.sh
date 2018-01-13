#!/bin/bash

# drop schema public
/usr/bin/psql -U postgres -d reario_db -h linux6 -p 5432 -c "drop schema public cascade" > /dev/null 2>&1

# recreate schema public
/usr/bin/psql -U postgres -d reario_db -h linux6 -p 5432 -c "create schema public AUTHORIZATION reario" > /dev/null 2>&1

/usr/bin/psql -U postgres -d reario_db -h linux6 -p 5432 -c "create extension tablefunc with schema public" > /dev/null 2>&1


# dump tables
# echo "dumping energia...."
/usr/bin/pg_dump -t public.energia --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h linux6 -p 5432
echo "dumping events....."
/usr/bin/pg_dump -t public.events --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h linux6 -p 5432
echo "dumping ss...."
/usr/bin/pg_dump -t public.ss --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h linux6 -p 5432

# dump views
echo "dumping view event distribution...."
/usr/bin/pg_dump -t public.event_distribution --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h linux6 -p 5432
echo "dumping view event_distribution_old...."
/usr/bin/pg_dump -t public.event_distribution_old --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h linux6 -p 5432

