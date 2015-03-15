#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
// #include <mysql/mysql.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "libpq-fe.h"

const char conninfo[]="hostaddr=192.168.1.103 user=reario dbname=reario_db";
PGconn *conn;
char *logdbname="/home/reario/log/dblog";
char errormsg[256];

void logdbvalue(char *filename, char *message)
{
  /*scrive su filename il messaggio message*/
  FILE *logfile;
  logfile=fopen(filename,"a");
  if(!logfile) return;
  fprintf(logfile,"pg %s\n",message);
  fclose(logfile);
}

int init_ss_table (char *d) {
  uint16_t i;
  char query_insert[256];
  char query_delete[256];
  PGresult   *res;
  conn = PQconnectdb(conninfo);
  if (PQstatus(conn) != CONNECTION_OK)
    {
      sprintf(errormsg,"init_ss Error %s\n", PQerrorMessage(conn));
      PQfinish(conn);
      //logdbvalue(logdbname,errormsg);
      return (1);
    } 


  sprintf(query_delete,"DELETE FROM ss where data='%s';",d);      
  res=PQexec(conn, query_delete);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) { 
    sprintf(errormsg,"init_ss after query %s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    logdbvalue(logdbname,errormsg);  
    return (1);
  }  
  for (i=0;i<=23;i++) {
    sprintf(query_insert,"INSERT INTO ss (data,ora,secA,secP,scaA,scaP) VALUES (CURRENT_DATE,'%02d:30:00',0,0,0,0);",i);       
    res=PQexec(conn, query_insert);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) { 
      sprintf(errormsg,"init_ss Error after query %s\n", PQerrorMessage(conn));
      PQclear(res);
      PQfinish(conn);
      logdbvalue(logdbname,errormsg);  
      return (1);
    }
  }
  sprintf(errormsg,"init ss OK");
  logdbvalue(logdbname,errormsg);
  PQclear(res);
  PQfinish(conn);
  return(0);
}

int update (char *table, char *data, uint16_t ora, uint16_t valori1[], uint16_t valori2[])
{
  char query[256];
  uint16_t i;  
  PGresult   *res;

  init_ss_table(data);
  conn = PQconnectdb(conninfo);
  if (PQstatus(conn) != CONNECTION_OK)
    {
      sprintf(errormsg,"scatti/secondi Error %s\n", PQerrorMessage(conn));
      PQfinish(conn);
      logdbvalue(logdbname,errormsg);
      return (1);
    }

  for (i=0;i<=ora;i++) {

    sprintf(query,"UPDATE %s SET secA=%d, secP=%d,scaA=%d,scaP=%d WHERE data='%s' AND ora='%d:30:00'", table, valori1[i], valori1[i+24], valori2[i], valori2[i+24],data,i);

    /*sprintf(query,"INSERT INTO %s (data, ora, autoc, pozzo) VALUES ('%s','%s','%02d:30:00','%d','%d') ON DUPLICATE KEY UPDATE autoc=%d, pozzo=%d;", mysql_table, data, i, valori[i], valori[i+24], valori[i], valori[i+24] );*/

    res=PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) { 
      sprintf(errormsg,"scatti/secondi Error %s\n", PQerrorMessage(conn));
      PQclear(res);
      PQfinish(conn);
      logdbvalue(logdbname,errormsg);  
      return (1);
    }

    /* else { */
    /*   sprintf(errormsg,"scatti/secondi %s INSERT statement succeeded", data); */
    /*   logdbvalue(logdbname,errormsg); */
    /* } */
    PQclear(res);    
  }
  PQfinish(conn);
  return(0);  
}

int insert (struct tm *ptr, float V, float A, float P1, float P2, float P3, float kWh)
{
  char datestring[11];
  char timestring[9];
  char query[256];
  PGresult   *res;

  strftime(datestring, sizeof(datestring),"%Y-%m-%d",ptr);
  strftime(timestring, sizeof(timestring),"%H:%M:%S",ptr);

  conn = PQconnectdb(conninfo);

  sprintf(query,"INSERT INTO energia (data,ora,volt,ampere,kW,kWPT,kWPOZZO,kWh) VALUES ('%s','%s','%3.3f','%3.2f','%3.2f','%3.2f','%3.2f','%3.0f');",datestring,timestring,V,A,P1,P2,P3,kWh);  

  if (PQstatus(conn) != CONNECTION_OK) 
    {
      sprintf(errormsg,"Error %s\n", PQerrorMessage(conn));
      PQfinish(conn);
      logdbvalue(logdbname,errormsg);  
      return (1);
    }
  res=PQexec(conn, query);

  if (PQresultStatus(res) != PGRES_COMMAND_OK) { 
    sprintf(errormsg,"Error %s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    logdbvalue(logdbname,errormsg);  
    return (1);
  }
  else { 
    sprintf(errormsg,"%s %s INSERT statement succeeded", datestring, timestring);
    logdbvalue(logdbname,errormsg);
  }
  PQclear(res);
    PQfinish(conn);
    return(0);
}
