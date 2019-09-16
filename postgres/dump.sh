#!/bin/bash 
PGPASSWORD=ipadsmf01
export PGPASSWORD
#HOST=raspi3
HOST=$1
export HOST


if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    exit
fi


echo $HOST


# drop schema public
/usr/bin/psql -U postgres -d reario_db -h $HOST -p 5432 -c "drop schema public cascade" > /dev/null 2>&1

# recreate schema public
/usr/bin/psql -U postgres -d reario_db -h $HOST -p 5432 -c "create schema public AUTHORIZATION reario" > /dev/null 2>&1

# install tablefunc function
/usr/bin/psql -U postgres -d reario_db -h $HOST -p 5432 -c "create extension tablefunc with schema public" > /dev/null 2>&1

######################################################################
# dump tables
echo "dumping energia...."
/usr/bin/pg_dump -t public.energia --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432
echo "dumping events....."
/usr/bin/pg_dump -t public.events --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432
echo "dumping sample...."
/usr/bin/pg_dump -t public.vaw --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432
echo "dumping ss...."
/usr/bin/pg_dump -t public.ss --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432

######################################################################
# dump views
echo "dumping view event distribution...."
/usr/bin/pg_dump -t public.event_distribution --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432
echo "dumping view event_distribution_old...."
/usr/bin/pg_dump -t public.event_distribution_old --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432
echo "dumping view ToOn...."
/usr/bin/pg_dump -t public.MaxDailyOn --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432
echo "dumping view NightScatti...."
/usr/bin/pg_dump -t public.nightscatti --clean --if-exists -h localhost -d reario_db --role=reario | /usr/bin/psql -U reario -d reario_db -h $HOST -p 5432
######################################################################
