/*
 * Fa un campionamento ogni WAIT secondi (definito dentro sample.h) e inserisce i dati dentro il DB nella tabella VAW
 * Viene eseguito come daemo e scrive un log degli errori DB in ./energia
 */

#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sysexits.h>
#include <netinet/in.h>
#include <modbus.h>
#include <time.h>

#include "libpq-fe.h"
#include "sample.h"


const char conninfo[]=PG_CONNINFO;
char *logdbname=PG_LOG_FILE;
char errormsg[256];
PGconn *conn;
char logname[50];
char *dat[4]; /*array di puntatori alle stringhe che rappresentano il timestamp*/
char *table=TABLE;

/*************************************/
void logdbvalue(char *filename, char *message)
{
  /*scrive su filename il messaggio message*/
  FILE *logfile;
  logfile=fopen(filename,"a");
  if(!logfile) return;
  fprintf(logfile,"pg %s\n",message);
  fclose(logfile);
}

//int insert (char *table, struct tm *ptr, float V, float A, float P1, float P2, float P3, float kWh)
int insert (char *table, float V, float A, float kW, float kWh)
{

  char query[256];
  PGresult   *res;

  conn = PQconnectdb(conninfo);

  sprintf(query,"INSERT INTO %s (ts,volt,ampere,kW,kWh) VALUES (now(),'%3.2f','%3.2f','%3.2f','%3.2f');",table,V,A,kW,kWh);  

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
  PQclear(res);
    PQfinish(conn);
    return(0);
}

/*************************************/


struct tm  *date_string(char **d) {
  
  char giorno[3];
  char mese[4];
  char anno[5];
  char ora[9];
  
  struct tm *ptr;
  time_t tm;
  
  /* prende la data */
  tm = time(NULL);
  ptr = localtime(&tm);
  /* ATTENZIONE: settare la variabile TZ per impostare il corretto TimeZone: per esempio TZ=":Europe/Rome" nel .bash_profile */
  tzset(); /*inizializza prendendo la variabile di sistema $TZ impostata come variabile ambiente*/
  (ptr->tm_hour)=(ptr->tm_hour);

  strftime(giorno, sizeof(giorno),"%d",ptr);
  strftime(mese, sizeof(mese),"%b",ptr);
  strftime(anno, sizeof(anno),"%Y",ptr);
  strftime(ora, sizeof(ora),"%H.%M.%S",ptr);

  d[GIORNO]=strdup(giorno);
  d[MESE]=strdup(mese);
  d[ANNO]=strdup(anno);  
  d[ORA]=strdup(ora);
  return(ptr);    
}


void logvalue(char *filename, char *message)
{
  /*scrive su filename il messaggio message*/
  FILE *logfile;
  logfile=fopen(filename,"a");
  if(!logfile) return;
  fprintf(logfile,"%s\n",message);
  fclose(logfile);
}

void signal_handler(int sig)
{
  switch(sig) {
  case SIGHUP:
    logvalue(LOG_FILE,"ATT: Reset Modbus connection\n");
    break;
  case SIGTERM:
    logvalue(LOG_FILE,"terminate signal catched");
    exit(EX_OK);
    break;
  }
}

void daemonize()
{
  int i,lfp;
  char str[10];
  if(getppid()==1) return; /* already a daemon */
  i=fork();
  if (i<0) exit(EX_OSERR); /* fork error */
  if (i>0) exit(EX_OK); /* parent exits */
  /* child (daemon) continues */
  setsid(); /* obtain a new process group */
  for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
  i=open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */
  umask(027); /* set newly created file permissions */
  // chdir(RUNNING_DIR); /* change running directory */
  lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
  if (lfp<0) exit(EX_OSERR); /* can not open */
  if (lockf(lfp,F_TLOCK,0)<0) exit(EX_OK); /* can not lock */
  /* first instance continues */
  snprintf(str,(size_t)10,"%d\n",getpid());
  write(lfp,str,strlen(str)); /* record pid to lockfile */
  signal(SIGCHLD,SIG_IGN); /* ignore child */
  signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
  signal(SIGTTOU,SIG_IGN);
  signal(SIGTTIN,SIG_IGN);
  signal(SIGPIPE,SIG_IGN);
  signal(SIGHUP,signal_handler); /* catch hangup signal */
  signal(SIGTERM,signal_handler); /* catch kill signal */
}


int main()
     
{
  daemonize();
  
  //struct hostent *hp; /* = gethostbyname(argv[1]); */
  char record[60];

  modbus_t *mb;
  uint16_t tab_reg[68];
  int addr=7; /* indirizzo PLC da dove iniziare a leggere */
  int nregs=69; /* 0..68 */

  int PATTIVAL, PATTIVAH,I1L, I1H, VINH, VINL,kWhH,kWhL;
  float PATTIVA, VOLT, AMPERE, kWh;
  

  //struct tm *ptr;
  //time_t tm; 

  /* run */
  while(1) {
 
    /*************************************/

    mb = modbus_new_tcp( HOST, PORT);      
    
    if ( modbus_connect(mb) == -1) {
      /*Errore di comunicazione*/
      date_string(dat); /* imposto il timestamp per il log */
      snprintf(record,(size_t)60,"%s-%s-%s;%s;%s",dat[GIORNO],dat[MESE],dat[ANNO],dat[ORA],"Communication error");
      logvalue(LOG_FILE,record); /* scrivo su file di log l'errore */
      modbus_free(mb); /* ripulisco il context */
    } else { /* connessione OK, vado a leggere i registri */
      
      /****************************************/
      /* Reads n registers from the address 0 */
      /****************************************/
      modbus_read_registers(mb, addr, nregs, tab_reg);      

      /***********************/
      /* TENSIONE e CORRENTE */
      /***********************/
      VINL=tab_reg[22-7];
      VINH=tab_reg[21-7];
      I1L=tab_reg[8-7];
      I1H=tab_reg[7-7];
      PATTIVAL=tab_reg[30-7];
      PATTIVAH=tab_reg[29-7];
      kWhL=tab_reg[46-7];
      kWhH=tab_reg[45-7];

      VOLT=(float)(VINL+(VINH<<16))/1000;
      AMPERE=(float)(I1L+(I1H<<16))/1000;
      PATTIVA=(float)(PATTIVAL+(PATTIVAH<<16))/100;
      kWh=(float)(kWhL+(kWhH<<16));

      if (VOLT>0 && AMPERE>0) { /* valori non nulli, quindi vado ad inserire nel DB */
	if ( insert(table,VOLT,AMPERE,PATTIVA,kWh)<0) {
	  logvalue(LOG_FILE,"Errore di inserimento record nel DB"); 
	}
      } else { /* ho letto dei valori nulli di volt e ampere*/
	logvalue(LOG_FILE,"Letti valori nulli per volt e ampere"); 
      };


      modbus_close(mb);
      modbus_free(mb);

    }

    sleep(WAIT); /*attendo WAIT secondi. Anche se c'e' stato errore di connessione*/
  }
    return 0;
}

/* EOF */
