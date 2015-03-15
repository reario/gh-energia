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


extern int insert (struct tm *ptr, float V, float A, float P1, float P2, float P3, float kWh);

#include "gh.h"

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

void set_logname(char **d) {
  snprintf(logname,(size_t)50,"%s%s-%s-%s",RUNNING_DIR,d[GIORNO],d[MESE],d[ANNO]);
}

int main()
     
{
  daemonize();
  
  struct hostent *hp; /* = gethostbyname(argv[1]); */
  char record[60];

  char curl1[]="/usr/bin/curl -v -H \"Content-Type: application/json\" -X POST -d ";
  char json[220];
  char curl2[]=" http://api.sen.se/events/\?sense_key=0sA_vSIOwv8k9WRj9gsZlQ";
  char curl[420];

  modbus_t *mb;
  uint16_t tab_reg[68];
  int addr=7; /* indirizzo PLC da dove iniziare a leggere */
  int nregs=69; /* 0..68 */

  int PATTIVAL, PATTIVAH,I1L, I1H, VINH, VINL,kWhH,kWhL;
  float PATTIVA, VOLT, AMPERE, kWh;
  
  /* PM9 derivati */
  int PATTIVAL_PT, PATTIVAH_PT, PATTIVAL_POZZO, PATTIVAH_POZZO;
  float PATTIVA_PT, PATTIVA_POZZO;
  uint16_t tab_pattiva_pt[3];
  uint16_t tab_pattiva_pozzo[3];

  struct tm *ptr;
  //time_t tm; 


  /* run */
  while(1) {
 
    /*************************************/
    hp=gethostbyname(HOST); /* lo ricalcolo sempre perche' l'ip del GH potrebbe cambiare */
    mb = modbus_new_tcp( (char*)inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0])), PORT);      
    
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

      /*PM9 derivati*/
      modbus_read_registers(mb, 329, 2, tab_pattiva_pt);
      modbus_read_registers(mb, 429, 2, tab_pattiva_pozzo);            

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

      /* PM9 derivati */
      PATTIVAL_PT=tab_pattiva_pt[1];
      PATTIVAH_PT=tab_pattiva_pt[0];
      PATTIVA_PT=(float)(PATTIVAL_PT+(PATTIVAH_PT<<16))/100;

      PATTIVAL_POZZO=tab_pattiva_pozzo[1];
      PATTIVAH_POZZO=tab_pattiva_pozzo[0];
      PATTIVA_POZZO=(float)(PATTIVAL_POZZO+(PATTIVAH_POZZO<<16))/100;

      /*****************************************************************************************/ 
      /* reimposto comunque il nome del file di log per gestire il cambio del giorno.          */
      /* il confronto con il giorno della precedente iterazione sarebbe stato piu' complicato. */
      /*****************************************************************************************/ 
      
      ptr=date_string(dat);
      set_logname(dat);

#ifdef _DB_
      if ( insert(ptr,VOLT,AMPERE,PATTIVA,PATTIVA_PT,PATTIVA_POZZO,kWh)<0) {
	logvalue(LOG_FILE,"Errore di inserimento record nel DB"); 
      };
#endif
      snprintf(record,(size_t)60,"%s-%s-%s;%s;%3.3f;%3.2f;%3.2f;%3.2f;%3.2f,%3.0f",
	       dat[GIORNO],dat[MESE],dat[ANNO],dat[ORA],
	       VOLT,AMPERE,PATTIVA,PATTIVA_PT,PATTIVA_POZZO,kWh); 

      logvalue(logname,record);

      snprintf(json,
	       (size_t)220,
      "'[{\"feed_id\":24257,\"value\":%3.3f},{\"feed_id\":24261,\"value\":%3.3f},{\"feed_id\":24262,\"value\":%3.3f},{\"feed_id\":24263,\"value\":%4.0f}]'",
	       VOLT,AMPERE,PATTIVA,kWh);

      snprintf(curl,
	       (size_t)420,
	       "%s%s%s",curl1,json,curl2);
      /* logvalue(LOG_FILE,curl); */
      modbus_close(mb);
      modbus_free(mb);

    }
    //    system(curl); 
    system("/home/reario/energia/ss 2>&1");

    sleep(60); /*attendo 60 secondi. Anche se c'e' stato errore di connessione*/


    /* le due routine sotto sono state sostituite da una unica routine 
       che popola direttamente una tabella SS e non due tabelle scatti && secondi
      system("/home/reario/energia/scatti 2>&1");
      system("/home/reario/energia/secondi 2>&1");
    */

#ifdef _PLOT_
    system("/home/reario/energia/value.sh 2>&1");
    system("/home/reario/energia/scatti.sh 2>&1");
    system("/home/reario/energia/secondi.sh 2>&1");
    system("/home/reario/energia/ss.sh 2>&1");
    system(curl); 
   /*
    curl -v -H "Content-Type: application/json" -X POST -d '[{"feed_id": 24261,"value": 10.4},{"feed_id": 24262, "value": 3}]' http://api.sen.se/events/\?sense_key=0sA_vSIOwv8k9WRj9gsZlQ
    system("/usr/bin/curl 2>&1");
    */
#endif

  }
    return 0;
}

/* EOF */
