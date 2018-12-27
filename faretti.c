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
  // ^ mette a 1 se i valori sono diversi, mette a 0 se sono uguali: in sostanza inverte lo stato del bit-esimo bit di reg  
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
  int rflag,sflag;
  char *rstate,*sstate;

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
	  
	    qui va la gestione dei parametri a riga di comando dati da ch	    
	   */
      /*****************************/
      while ( (ch = getopt (argc, argv, "r:s:")) != -1) {
	switch (ch) {
	case 'r':
	  rflag=1;
	  rstate = optarg;
	  break;
	case 's':
	  //---------------------------------------------
	  sflag=1;
	  sstate = optarg;	  
	  break;
	default:
	  printf("--------\n");
	  abort();
	}
      }
      //*****************************SOPRA ********************************************************************************
      if (rflag) {
	printf("--- sopra: r flag block\n");
	  if (rstate == NULL) {
	    printf("Stato faretti SOPRA (r option) non specificato. Ignoro\n");
	  } else {
	    if ( strcmp(rstate,"ON") == 0 ) {
	      printf("vuoi faretti sopra a ON\n");
	      // ------- 1 controllo se lo stato che vuoi è già impostato
	      // ------- 1.1 se lo stato è uguale, allora non faccio nulla
	      if (otb_in[0] & (1<<FARI_ESTERNI_IN_SOPRA) ) { 
		printf("Lo stato dei faretti sopra è già a ON\n");
	      } else {
	      // ------- 2 lo stato è differente e quindi faccio lo switch
	      // ------- 2.1 controllo se lo switch ha funzionato andando a vedere il rispettivo ingresso
		if (
		  modbus_write_register(mb_otb,100,otb_out[0]^(1<<FARI_ESTERNI_SOPRA)) !=-1

		  //interruttore(mb_otb,FARI_ESTERNI_SOPRA,otb_out[0]) >= 0
		) { // metto a 1 Q0
		  otb_out[0]=otb_out[0]^(1<<FARI_ESTERNI_SOPRA);
		  printf("fari esterni SOPRA cambiato stato confermato a: %s\n", otb_in[0] & (1<<FARI_ESTERNI_IN_SOPRA)?"OFF":"ON");
		} else {
		  printf("Errore nel cambio stato da OFF a ON");
		} 
	      }
	    }
	    else {
	      if ( strcmp(rstate,"OFF") == 0 ) {
		printf("vuoi faretti sopra a OFF\n");
		// ------
	      // ------- 1 controllo se lo stato che vuoi è già impostato
	      // ------- 1.1 se lo stato è uguale, allora non faccio nulla
		if (! (otb_in[0] & (1<<FARI_ESTERNI_IN_SOPRA)) ) { 
		printf("Lo stato dei faretti sopra è già a OFF\n");
	      } else {
	      // ------- 2 lo stato è differente e quindi faccio lo switch
	      // ------- 2.1 controllo se lo switch ha funzionato andando a vedere il rispettivo ingresso
		  if (
		    modbus_write_register(mb_otb,100,otb_out[0]^(1<<FARI_ESTERNI_SOPRA)) !=-1
		    
		    //interruttore(mb_otb,FARI_ESTERNI_SOPRA,otb_out[0]) >= 0
		  ) { // metto a 1 Q0
		    otb_out[0]=otb_out[0]^(1<<FARI_ESTERNI_SOPRA);
		    printf("fari esterni SOPRA cambiato stato confermato a: %s\n", otb_in[0] & (1<<FARI_ESTERNI_IN_SOPRA)?"OFF":"ON");
		  } else {
		    printf("Errore nel cambio stato da ON a OFF");
		  } 
		}
	      }
	      
	      else {
		printf("non hai specificato lo stato che vuoi per i faretti sopra. specifica ON o OFF stato  on cambiato\n");
	      }
	    }
	  }
      }
      
      //***************************** SOTTO ****************************************************************
      if (sflag) {
	printf("--- sotto: s flag block\n");
	  if (sstate == NULL) {
	    printf("Stato faretti SOTTO (s option) non specificato. Ignoro\n");
	  } else {
	    if ( strcmp(sstate,"ON") == 0 ) {
	      printf("vuoi faretti sotto a ON\n");
	      // ------- 1 controllo se lo stato che vuoi è già impostato
	      // ------- 1.1 se lo stato è uguale, allora non faccio nulla
	      if (otb_in[0] & (1<<FARI_ESTERNI_IN_SOTTO) ) { 
		printf("Lo stato dei faretti sotto è già a ON\n");
	      } else {
	      // ------- 2 lo stato è differente e quindi faccio lo switch
	      // ------- 2.1 controllo se lo switch ha funzionato andando a vedere il rispettivo ingresso
		if (
		  modbus_write_register(mb_otb,100,otb_out[0]^(1<<FARI_ESTERNI_SOTTO)) != -1
		  
		  //interruttore(mb_otb,FARI_ESTERNI_SOTTO,otb_out[0]) >= 0
		) { // metto a 1 Q0
		  otb_out[0]=otb_out[0]^(1<<FARI_ESTERNI_SOTTO);
		  printf("fari esterni SOTTO cambiato stato confermato a: %s\n", otb_in[0] & (1<<FARI_ESTERNI_IN_SOTTO)?"OFF":"ON");
		} else {
		  printf("Errore nel cambio stato da OFF a ON");
		} 
	      }
	    }
	    else {
	      if ( strcmp(sstate,"OFF") == 0 ) {
		printf("vuoi faretti sotto a OFF\n");
		// ------
	      // ------- 1 controllo se lo stato che vuoi è già impostato
	      // ------- 1.1 se lo stato è uguale, allora non faccio nulla
		if (! (otb_in[0] & (1<<FARI_ESTERNI_IN_SOTTO)) ) { 
		printf("Lo stato dei faretti sopra è già a OFF\n");
	      } else {
	      // ------- 2 lo stato è differente e quindi faccio lo switch
	      // ------- 2.1 controllo se lo switch ha funzionato andando a vedere il rispettivo ingresso
		  if (
		    modbus_write_register(mb_otb,100,otb_out[0]^(1<<FARI_ESTERNI_SOTTO) ) != -1
		    
		    //interruttore(mb_otb,FARI_ESTERNI_SOTTO,otb_out[0]) >= 0
		  ) { // metto a 1 Q0
		    otb_out[0]=otb_out[0]^(1<<FARI_ESTERNI_SOTTO);
		    printf("fari esterni SOTTO cambiato stato confermato a: %s\n", otb_in[0] & (1<<FARI_ESTERNI_IN_SOTTO)?"OFF":"ON");
		  } else {
		    printf("Errore nel cambio stato da ON a OFF");
		  } 
		}
	      }
	      
	      else {
		printf("non hai specificato lo stato che vuoi per i faretti sotto. Specifica ON o OFF. Stato  on cambiato\n");
	      }
	    }
	  }
      }
      /*****************************/
      /*
	  while (( ch = getopt (argc, argv, "rs")) != -1) {
	    switch (ch) {
	    case 'r': 
	      printf("Faretti sopra\n");
	      if ( interruttore(mb_otb,FARI_ESTERNI_SOPRA,otb_out[0]) >= 0) { // metto a 1 Q0
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
      */
	
	  modbus_close(mb_otb);
	  modbus_free(mb_otb);
	  return 0;
  }
  }
}
