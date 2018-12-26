/*
  ./faretti -r : cambia lo stato dei faretti sopra
  ./faretti -s : cambia lo stato dei faretti sotto
  ./faretti -rs : cambia lo stato dei faretti sopra E sotto
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <errno.h>
#include <modbus.h>
#include "gh.h"


int pulsante(modbus_t *m,int bobina) {
  if ( modbus_write_bit(m,bobina,TRUE) != 1 ) {
    printf("ERRORE DI SCRITTURA:PULSANTE ON");
    return -1;
  }
  sleep(1);
  if ( modbus_write_bit(m,bobina,FALSE) != 1 ) {
    printf("ERRORE DI SCRITTURA:PULSANTE OFF");
    return -1;
  }
  return 0;
}

int interruttore(modbus_t *m, int bit, uint16_t reg) {
  /* set 1 il bit iesimo di reg */
  reg=reg^(1<<bit);
  
  if ( modbus_write_register(m,100,reg) != 1 ) {
    printf("ERRORE DI SCRITTURA:INTERRUTTORE ON");
    return -1;
  }
  return 0;
}

int main (int argc, char ** argv) {

  modbus_t *mb_otb;

  uint16_t otb_in[10];
  uint16_t otb_out[10];
  
  int ch;

  mb_otb = modbus_new_tcp("192.168.1.11",PORT);

  /* faccio la connessione */
  if ( (modbus_connect(mb_otb) == -1) )
    {
      printf("ERRORE non riesco a connettermi con OTB\n");
      exit(1);
    } else {
    if ( (modbus_read_registers(mb_otb, 0, 3, otb_in) < 0) ||
	 (modbus_read_registers(mb_otb, 100, 3, otb_out) < 0)
    ) 
      {
	printf("Read Error %s\n",modbus_strerror(errno));
	exit(1);
      } else { /* non ci sono stati errori di comunicazione e di lettura 
	  /*
	    qui va la gestione dei parametri a riga di comando dati da ch	    
	   */
	  while (( ch = getopt (argc, argv, "rs")) != -1) {
	    switch (ch) {
	    case 'r': 
	      printf("Faretti sopra\n");
	      if ( interruttore(mb_otb,FARI_ESTERNI_SOPRA,otb_out[0]) >= 0) {
                printf("fari esterni SOPRA cambiato stato\n");  }
	      otb_in[0] & (1<<FARI_ESTERNI_IN_SOPRA) ? printf("fari esterni SOPRA sono OFF\n"):printf("fari esterni SOPRA sono ON\n");
	      break;
	    case 's':
	      printf("Faretti sotto\n");
              if ( interruttore(mb_otb,FARI_ESTERNI_SOTTO,otb_out[0]) >= 0) {
                printf("fari esterni SOTTO cambiato stato\n");  }
	      otb_in[0] & (1<<FARI_ESTERNI_IN_SOPRA) ? printf("fari esterni SOTTO sono OFF\n"):printf("fari esterni SOTTO sono ON\n");	      
	      break;
	    default:
	      printf("--------\n");
	      abort();
	    }
	  }
	
	  modbus_close(mb_otb);
	  modbus_free(mb_otb);
	  return 0;
    }
  }
}
