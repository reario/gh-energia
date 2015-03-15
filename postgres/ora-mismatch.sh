#!/bin/bash

#date +"%Y%m%d"
psql -c "select extract(hour from ora), extract(minute from ora) from energia where data='`date +"%Y%m%d"`';" -h 192.168.1.102 -U reario -d reario_db -At -F ':' > /tmp/ora-debian2
psql -c "select extract(hour from ora), extract(minute from ora) from energia where data='`date +"%Y%m%d"`';" -U reario -d reario_db -At -F ':' > /tmp/ora-debian3
diff -y -T /tmp/ora-debian2 /tmp/ora-debian3

date +"%Y%m%d"
