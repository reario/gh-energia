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

int bcd2int(uint16_t bcd)
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

  printf("\n%s, %02d/%02d/%d, ore %02i:%02i:%02i\n\n",giornisettimana[bcd2int(valori[0]-1)], bcd2int(giorno),bcd2int(mese),bcd2int(valori[4]), bcd2int(ore),bcd2int(min),bcd2int(valori[1]));

 return 0;
}

int main (int argc, char ** argv) {

  modbus_t *mb;
  struct hostent *hp;

/* name resolution and connection to modbus device */
  hp=gethostbyname("192.168.1.157");
  mb = modbus_new_tcp( (char*)inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0])), PORT);
  /* faccio la connessione */
  if ( (modbus_connect(mb) == -1) ) {
    printf("Errore di connessione\n");
  } else {
    /* stampa la data */  
    readtime(mb);    
  } /* else esterno che wrappa il connect*/
  
 modbus_close(mb);
 modbus_free(mb);
 
 return 0;
}
