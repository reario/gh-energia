/**************************************************************
UNIX Daemon Server Programming Sample Program
Levent Karakas <levent at mektup dot at> May 2001

To compile:	cc -o exampled examped.c
To run:		./exampled
To test daemon:	ps -ef|grep exampled (or ps -aux on BSD systems)
To test log:	tail -f /tmp/exampled.log
To test signal:	kill -HUP `cat /tmp/exampled.lock`
To terminate:	kill `cat log/logvalue.lock`
**************************************************************/
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
#include "gh.h"


#ifdef _DB_
extern int insert (struct tm *ptr, float V, float A, float P1, float P2, float P3, float kWh);
extern int update (char *table, char *data, uint16_t ora, uint16_t valori1[],uint16_t valori2[]);
#endif

char logname[50];
char *dat[4]; /*array di puntatori alle stringhe che rappresentano il timestamp*/


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

void set_logname() {
  snprintf(logname,(size_t)50,"%s%s-%s-%s",RUNNING_DIR,dat[GIORNO],dat[MESE],dat[ANNO]);
}

int main()
     
{
  
  struct hostent *hp; /* = gethostbyname(argv[1]);*/
  char record[60];
  modbus_t *mb;

  
  uint16_t tab_secondi[50];
  int nregs_secondi=48; /* 0..47 num. registri da leggere*/
  int addr_secondi=151; /* indirizzo primo registro da leggere */

  uint16_t tab_scatti[50];
  int nregs_scatti=48;
  int addr_scatti=100;

  uint16_t tab_oraplc[10];
  int nregs_oraplc=1;
  int addr_oraplc=150;

  int i;
  
  date_string(dat);
  set_logname();
  
  /*************************************/
  hp=gethostbyname(HOST); /* lo ricalcolo sempre perche' l'ip del GH potrebbe cambiare */
  mb = modbus_new_tcp( (char*)inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0])), PORT);      

  if ( modbus_connect(mb) == -1) {
    /*Errore di comunicazione*/
    printf("ERRORE1\n");
    date_string(dat); /* imposto il timestamp per il log */
    snprintf(record,(size_t)60,"%s-%s-%s;%s;%s",dat[GIORNO],dat[MESE],dat[ANNO],dat[ORA],"Communication error");
    logvalue(LOG_FILE,record); /* scrivo su file di log l'errore */
    modbus_free(mb); /* ripulisco il context */
    exit(1);
  } else { /* connessione OK, vado a leggere i registri */
    
    /***********************************************/
    /* Reads nregs registers from the address addr */
    /***********************************************/
    if ( (modbus_read_registers(mb, addr_secondi, nregs_secondi, tab_secondi) == -1) ||
	 (modbus_read_registers(mb, addr_scatti,  nregs_scatti,  tab_scatti)  == -1) ||
	 (modbus_read_registers(mb, addr_oraplc,  nregs_oraplc,  tab_oraplc)  == -1) ) {
      /* Errore di lettura */
      printf("ERRORE2\n");
      date_string(dat); /* imposto il timestamp per il log */
      snprintf(record,(size_t)60,"%s-%s-%s;%s;%s",dat[GIORNO],dat[MESE],dat[ANNO],dat[ORA],"Communication error");
      logvalue(LOG_FILE,record); /* scrivo su file di log l'errore */
      modbus_close(mb);
      modbus_free(mb); /* ripulisco il context */
      exit(1);
    } else { /* non ci sono stati errori di comunicazione e di lettura */
      /***********************************************/
      /* stampo i registri letti                     */
      /***********************************************/
      /*ora;secondi_utoclave;secondi_pozzo;scatti_autoclave;scatti_pozzo */
      for (i=0;i<=tab_oraplc[0];i++) {
	printf("%d.30;%d;%d;%d;%d",i,tab_secondi[i],tab_secondi[i+24],tab_scatti[i],tab_scatti[i+24]);
	printf("\n");
      }
#ifdef _DB_

      /* 
	 record ora contiene la data in formato yyyymmdd e anche l'ora nel formato ora:30
      */
      
      strftime(record,sizeof(record),"%Y%m%d",date_string(dat));
      
      /*
	update("secondi", record, tab_oraplc[0], tab_secondi);
	update("scatti",  record, tab_oraplc[0], tab_scatti);
      */
      
      update("ss", record, tab_oraplc[0], tab_secondi, tab_scatti);

#endif        
      modbus_close(mb);
      modbus_free(mb);  
    }
    
  }
  return 0; 
}
/* EOF */
