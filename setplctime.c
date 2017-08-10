#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <modbus.h>
#include <arpa/inet.h>
#include "gh.h"
#include <time.h>

int sethour (modbus_t *m,int addr, int nreg, const uint16_t *valori) {
  
  if ( modbus_write_registers(m,addr,nreg,valori) != nreg ) {
    printf("ERRORE DI SCRITTURA hour\n");
    return -1;
  }
  return 0;
}

int bcd2int(unsigned int bcd)
{
  int pos = sizeof(bcd) * 8;
  int digit;
  int val = 0;

  do {
    pos -= 4;
    digit = (bcd >> pos) & 0xf;
    val = val * 10 + digit; 
  } while (pos > 0);

  return val;
}
int setsinglebit(modbus_t *m, int bobina, int valore) {
  if ( modbus_write_bit(m,bobina,valore) != 1 ) {
    printf("ERRORE DI SCRITTURA bit %i",bobina);
    return -1;
  }
  return 0;
}


int readtime (modbus_t *m) {
  int rc;
  uint16_t valori[5],oremin,ore,min,giornomese,giorno,mese;
  char *giornisettimana[]={"lunedì","martedì","mercoledì","giovedì","venerdì","sabato","domenica"};
  rc=modbus_read_registers(m,11,5,valori);
  if (  rc==-1 ) {
    printf("ERRORE DI LETTURA TIME\n");
    return -1;
  }
  oremin=valori[2];
  giornomese=valori[3];

  ore=(uint16_t)(oremin & (uint16_t)65280)>>(uint16_t)8;
  min=(uint16_t)(oremin & (uint16_t)255);

  mese=(uint16_t)(giornomese & (uint16_t)65280)>>(uint16_t)8;
  giorno=(uint16_t)(giornomese & (uint16_t)255);

  printf("Data PLC --> %s, %02d/%02d/%d, ore %02i:%02i:%02i\n",giornisettimana[bcd2int(valori[0]-1)], bcd2int(giorno),bcd2int(mese),bcd2int(valori[4]), bcd2int(ore),bcd2int(min),bcd2int(valori[1]));

 return 0;
}


int setval (modbus_t *m, int addr, int  valore) {
  if ( modbus_write_register(m,addr,valore) < 0 ) {
    printf("ERRORE DI SCRITTURA hour\n");
    return -1;
  }
  return 0;
}

int main (int argc, char ** argv) {

  modbus_t *mb;
  struct hostent *hp;

  /* int nregs=5;  */
  /* int addr=49;  */
    
  struct tm *ptr;
  time_t tm;

  /* ATTENZIONE: settare la variabile TZ per impostare il corretto TimeZone: per esempio TZ=":Europe/Rome" nel .bash_profile */
  tzset(); /*inizializza prendendo la variabile di sistema $TZ impostata come variabile ambiente*/

  /*
    RTC Functions: words containing current date and time values (in BCD): S and U, SIM 
    %SW49 xN Day of the week (N=1 for Monday) 
    %SW50 00SS Seconds
    %SW51 HHMM Hour and minute 
    %SW52 MMDD Month and day 
    %SW53 CCYY Century and year 

Esempio:
    Monday, 19 April, 2002 13:40:30
    
    %SW49 0001 Day of the week1 ( 1 = Monday, 2 = Tuesday, 3 = Wednesday, 4 = Thursday, 5 = Friday, 6 = Saturday, 7 = Sunday ) 
    %SW50 0030 Second 00SS 
    %SW51 1340 Hour Minute HHMM
    %SW52 0419 Month Day MMDD 
    %SW53 2002 Century Year CCYY 
 */

  /*
    see man localtime
    int tm_mday;
    int tm_mon;   
    int tm_year;  
    
    day_of_the_week=ptr->tm_wday;
    seconds=ptr->tm_sec;
    minutes=ptr->tm_min;
    hours=ptr->tm_hour
  */

  /* name resolution and connection to modbus device */
  hp=gethostbyname("192.168.1.157");
  mb = modbus_new_tcp( (char*)inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0])), PORT);
  /* faccio la connessione */
  if ( (modbus_connect(mb) == -1) ) {
    printf("Errore di connessione\n");
  } else {
    printf("Connesso\n");
    /* prende la data locale*/
    tm = time(NULL);
    ptr = localtime(&tm);
    readtime(mb);    
    setval(mb,76,ptr->tm_sec );
    setval(mb,77,ptr->tm_hour*100+ptr->tm_min ); /* ore:minuti es. 1252: ore 12 e 52 minuti*/
    printf("Data impostata --> %d/%d/%d %d\n",ptr->tm_mday,ptr->tm_mon+1,ptr->tm_year+1900,ptr->tm_hour*100+ptr->tm_min);

    /* 
       commit: on the rising edge it set %S50 and time is equal to the content of %MW76 and %MW77. With %S50=1 time is updatable. 
       On the falling edge it reset %S50. With %S50=0 time is no more updatable  
       Reset of %MW95 is done on PLC so is not necessary to set it to 0 here. This is why here we are only setting %M95 to TRUE. 
    */

    if (setsinglebit(mb,95,TRUE)==0) {;
      printf("SCRITTO\n");
      printf("----------------------------------------------\n");
    } else {
      printf("ERRORE nella scrittura del bit 95 sul PLC. Ora NON aggiornata");
    }
  } /* else esterno che wrappa il connect*/
  
 modbus_close(mb);
 modbus_free(mb);
 
 return 0;
}
