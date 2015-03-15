# mysqldump -h debian1 -ureario -prwAKt7hl reario_db energia --skip-triggers --compact --no-create-info|sed "s/\`//g">reario_db-only-insert.sql



echo "1)  svuoto la table ENERGIA dal server locale..."
psql -U reario reario_db -c "truncate table events"

echo "1.1) caricamento dei valori energia dal server principale al server locale...."
mysqldump -h insomnia247.nl -ureario -prwAKt7hl reario_db energia --skip-triggers --compact --no-create-info|sed "s/\`//g" | psql -U reario reario_db

echo "2.0) svuoto la table SS locale...."
psql -U reario reario_db -c "truncate table ss" && 

echo "2.1) collegamento al server principale e creazione delle tabelle a partire dalla vista....."
mysql -h insomnia247.nl -ureario -prwAKt7hl -e "drop table ss_table;create table ss_table as select * from ss;" --database reario_db && 

echo "2.2) esecuzione dump dal server principale e import sul server locale....."
mysqldump -h insomnia247.nl -ureario -prwAKt7hl reario_db ss_table --skip-triggers --compact --no-create-info|sed "s/\`ss_table\`/ss \(data,ora,seca,scaa,secp,scap\) /g"| psql -U reario reario_db
