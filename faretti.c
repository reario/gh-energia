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
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
int interruttore(modbus_t *m, uint16_t tab[], uint16_t reg, int bit) {
  /*
    tab[]: tabellone dei registri. 
    reg:   il registro su cui si vuole operar: contiene un numero che consideriamo come un array di bit
    bit:   all'interno della stringa di bit di tab[reg], indica su quale bit dobbiamo lavorare (cambiare stato
  */
  tab[reg]=tab[reg]^(1<<bit); // cambio stato (lo inverto rispetto a come si trova) al bit-esimo bit di tab[reg]
  if ( modbus_write_register(m,100,tab[reg]) != 1) { // lo scrivo sull'HW dell'OTB. 100 è il regitro delle uscite su OTB (Q0-Q7)
    tab[reg]=tab[reg]^(1<<bit);      // ripristino il registro a come era prima
    return -1;
  }
  return 0;
}

int operate(modbus_t *mb,
	    uint16_t in[],
	    uint16_t out[],
	    int16_t what_in, // if <0 doesn't check the input linked to the output  
	    uint16_t what_out, // output to modify
	    uint16_t addr_reg_in,
	    uint16_t addr_reg_out,
	    uint16_t num_reg_in_to_read,
	    uint16_t num_reg_out_to_write,
	    uint16_t num_reg_in,
	    uint16_t num_reg_out) {

  if (modbus_read_registers(mb, addr_reg_in, num_reg_in_to_read, in) == -1 || 
      modbus_read_registers(mb, addr_reg_out, num_reg_out_to_write, out) == -1) 
    {
      printf("Errore READ (IN or OUT) registro %d or %d OTB: %i\n",addr_reg_in,addr_reg_out,errno);
      return -1;
    }
  if (what_in >=0) {
    printf("INGRESSI PRIMA erano %s \n",read_single_state(in[num_reg_in],what_in)?"ON":"OFF");
  }
  /*********************************************************/
      if ( interruttore(mb,out,num_reg_out,what_out) == -1 ) {
	printf("Errore WRITE (IN or OUT) registro %d OTB: %i\n",addr_reg_out,errno);
	return -1;
      }
  /*********************************************************/
  // serve per far assestare gli ingressi al fine di leggeere un valore di questi congruente con quanto 
  // si è appena scritto
  // se non mettessimo lo sleep ci potremmo trovare che gli ingressi sono ancora allo stato precedente 
  sleep(1);
  if (modbus_read_registers(mb, addr_reg_in, num_reg_in_to_read, in) == -1 || 
      modbus_read_registers(mb, addr_reg_out, num_reg_out_to_write, out) == -1)
    {
      printf("Errore READ (IN or OUT) registro %i or %i OTB: %i\n",addr_reg_in,addr_reg_out,errno);
      return -1;
    }
  if (what_in >=0) {
    printf("INGRESSI DOPO sono   %s \n",read_single_state(in[addr_reg_in],what_in)?"ON":"OFF");
  }
  return 0;
}
  
  
int main (int argc, char ** argv) {
  
  modbus_t *mb_otb;
  
  uint16_t otb_in[10];
  uint16_t otb_out[10];
  
  int ch;
  int rflag=0,sflag=0,aflag=0,nflag=0,fflag=0;
  // char *rstate,*sstate;

  mb_otb = modbus_new_tcp("192.168.1.11",PORT);

  /* faccio la connessione */
  if ( (modbus_connect(mb_otb) == -1) )
    {
      printf("ERRORE non riesco a connettermi con OTB\n");
      exit(1);
    } 

  while ( (ch = getopt (argc, argv, "afnrs")) != -1) {
    switch (ch) {
    case 'r':
      rflag=1;
      break;
    case 's':
      //---------------------------------------------
      sflag=1;
      break;
    case 'a':
      //---------------------------------------------
      aflag=1;
      break;
    case 'f':
      //---------------------------------------------
      fflag=1;
      break;
    case 'n':
      //---------------------------------------------
      nflag=1;
      break;
    default:
      printf("--------\n");
      abort();
    }
  }
  /*****************************SOPRA*************************************/
  if (rflag == 1) {
    modbus_read_registers(mb_otb, 0, 1, otb_in);    
    if (fflag == 1) {
      if ( read_single_state(otb_in[0],FARI_ESTERNI_IN_SOPRA) ) { 
      /* è acceso ed ho il flag f quindi lo spengo commutando */
	if (operate(mb_otb, // mb
		    otb_in, // in array di registri INGRESSI (Analogici, Digitali)
		    otb_out, // out array di registri USCITE (Analogiche, Digitali)
		    FARI_ESTERNI_IN_SOPRA, //what_in numero bit nel registro INGRESSI(se<0 non check gli ingressi)
		    FARI_ESTERNI_SOPRA, // what_out numero del bit all'interno del registro USCITE (bit da modificare)
		    0, // addr_reg_in registo partenza degli INGRESSO sul device
		    100, // addr_reg_out registo partenza delle USCITE sul device
		    3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		    3, // num_reg_in_to_write numero registri in USCITA da leggere
		    0, // numero del registro in INGRESSO interessato alla operazione
		    0 // numero del registro in USCITA interessato alla operazione
		    
	) != 0 ) {
	  exit(-1);
	} // operate
	
      } else {
	printf("Il faretto SOPRA era già spento e lo lascio spento perchè ho il flag -f\n");
      }
    } // fflag == 1
  
    if (nflag == 1) {
      if ( (! read_single_state(otb_in[0],FARI_ESTERNI_IN_SOPRA)) ) { 
	/* è spento ed ho il flag n quindi lo accendo commutando */
	if (operate(mb_otb, // mb
		    otb_in,  // in array di registri INGRESSI (Analogici, Digitali)
		    otb_out, // out array di registri USCITE (Analogiche, Digitali)
		    FARI_ESTERNI_IN_SOPRA, //what_in numero bit nel registro INGRESSI(se < 0 non check gli ingressi)
		    FARI_ESTERNI_SOPRA, // what_out numero del bit all'interno del registro USCITE (bit da modificare)
		    0, // addr_reg_in registo partenza degli INGRESSO sul device
		    100, // addr_reg_out registo partenza delle USCITE sul device
		    3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		    3, // num_reg_in_to_write numero registri in USCITA da leggere
		    0, // numero del registro in INGRESSO interessato alla operazione
		    0 // numero del registro in USCITA interessato alla operazione
		    
	) != 0 ) {
	  exit(-1);
	} // operate
      } else {
	printf("Il faretto SOPRA era già acceso e lo lascio acceso perchè ho il flag -n\n");
      }
    } // nflag == 1
  
    if ( (nflag != 1) && (fflag != 1) ) {
      /*non ho ne n ne f flag e quindi commuto*/
	if (operate(mb_otb, // mb
		    otb_in, // in array di registri INGRESSI (Analogici, Digitali)
		    otb_out, // out array di registri USCITE (Analogiche, Digitali)
		    FARI_ESTERNI_IN_SOPRA, //what_in numero bit nel registro INGRESSI(se<0 non check gli ingressi)
		    FARI_ESTERNI_SOPRA, // what_out numero del bit all'interno del registro USCITE (bit da modificare)
		    0, // addr_reg_in registo partenza degli INGRESSO sul device
		    100, // addr_reg_out registo partenza delle USCITE sul device
		    3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		    3, // num_reg_in_to_write numero registri in USCITA da leggere
		    0, // numero del registro in INGRESSO interessato alla operazione
		    0 // numero del registro in USCITA interessato alla operazione
		    
	) != 0 ) {
	  exit(-1);
	} // operate      
    }

  } // rflag == 1 

  /******************************************************************/

  ////////////////////////////////////////////////////
  if (sflag == 1) {
    modbus_read_registers(mb_otb, 0, 1, otb_in);    
    if (fflag == 1) {
      if ( read_single_state(otb_in[0],FARI_ESTERNI_IN_SOTTO) ) { 
      /* è acceso ed ho il flag f quindi lo spengo commutando */
	if (operate(mb_otb, // mb
		    otb_in, // in array di registri INGRESSI (Analogici, Digitali)
		    otb_out, // out array di registri USCITE (Analogiche, Digitali)
		    FARI_ESTERNI_IN_SOTTO, //what_in numero bit nel registro INGRESSI(se<0 non check gli ingressi)
		    FARI_ESTERNI_SOTTO, // what_out numero del bit all'interno del registro USCITE (bit da modificare)
		    0, // addr_reg_in registo partenza degli INGRESSO sul device
		    100, // addr_reg_out registo partenza delle USCITE sul device
		    3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		    3, // num_reg_in_to_write numero registri in USCITA da leggere
		    0, // numero del registro in INGRESSO interessato alla operazione
		    0 // numero del registro in USCITA interessato alla operazione
		    
	) != 0 ) {
	  exit(-1);
	} // operate
	
      } else {
	printf("Il faretto SOTTO era già spento e lo lascio spento perchè ho il flag -f\n");
      }
    } // fflag == 1
  
    if (nflag == 1) {
      if ( (! read_single_state(otb_in[0],FARI_ESTERNI_IN_SOTTO)) ) {
      /* è spento ed ho il flag n quindi lo accendo commutando */
	if (operate(mb_otb, // mb
		    otb_in, // in array di registri INGRESSI (Analogici, Digitali)
		    otb_out, // out array di registri USCITE (Analogiche, Digitali)
		    FARI_ESTERNI_IN_SOTTO, //what_in numero bit nel registro INGRESSI(se<0 non check gli ingressi)
		    FARI_ESTERNI_SOTTO, // what_out numero del bit all'interno del registro USCITE (bit da modificare)
		    0, // addr_reg_in registo partenza degli INGRESSO sul device
		    100, // addr_reg_out registo partenza delle USCITE sul device
		    3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		    3, // num_reg_in_to_write numero registri in USCITA da leggere
		    0, // numero del registro in INGRESSO interessato alla operazione
		    0 // numero del registro in USCITA interessato alla operazione
		    
	) != 0 ) {
	  exit(-1);
	} // operate
      } else {
	printf("Il faretto SOTTO era già acceso e lo lascio acceso perchè ho il flag -n\n");
      }
    } // nflag == 1
  
    if ( (nflag != 1) && (fflag != 1) ) {
      /*non ho ne n ne f flag e quindi commuto*/
	if (operate(mb_otb, // mb
		    otb_in, // in array di registri INGRESSI (Analogici, Digitali)
		    otb_out, // out array di registri USCITE (Analogiche, Digitali)
		    FARI_ESTERNI_IN_SOTTO, //what_in numero bit nel registro INGRESSI(se<0 non check gli ingressi)
		    FARI_ESTERNI_SOTTO, // what_out numero del bit all'interno del registro USCITE (bit da modificare)
		    0, // addr_reg_in registo partenza degli INGRESSO sul device
		    100, // addr_reg_out registo partenza delle USCITE sul device
		    3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		    3, // num_reg_in_to_write numero registri in USCITA da leggere
		    0, // numero del registro in INGRESSO interessato alla operazione
		    0 // numero del registro in USCITA interessato alla operazione
		    
	) != 0 ) {
	  exit(-1);
	} // operate      
    } //nflag !=1 && ...

  } // sflag == 1 

  ////////////////////////////////////////////////////
  /*
  if (sflag == 1) {
    if (operate(mb_otb, // modbus context
		otb_in, // in array di registri INGRESSI (Analogici, Digitali)
		otb_out, // out array di registri USCITE (Analogiche, Digitali)
		FARI_ESTERNI_IN_SOTTO, // what_in numero bit all'interno del registro INGRESSI
		FARI_ESTERNI_SOTTO, // what_out numero del bit all'interno del registro USCITE 
		0, // addr_reg_in registo partenza degli INGRESSO sul device
		100, // addr_reg_out registo partenza delle USCITE sul device
		3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		3, // num_reg_in_to_write numero registri in USCITA da leggere
		0, // numero del registro in INGRESSO interessato alla operazione
		0 // numero del registro in USCITA interessato alla operazione	
    ) != 0 ) {
      exit(-1);
    } 
  } 
  */
  /******************************************************************/
  if (aflag == 1) {
    printf("--- aflag ---\n");
    if (operate(mb_otb, // modbus contex
		otb_in, // in array di registri INGRESSI (Analogici, Digitali)
		otb_out, // out array di registri USCITE (Analogiche, Digitali)
		-1, // what_in numero bit all'interno del registro INGRESSI
		OTB_Q7, // what_out numero del bit all'interno del registro USCITE 
		0, // addr_reg_in registo partenza degli INGRESSO sul device
		100, // addr_reg_out registo partenza delle USCITE sul device
		3, // num_reg_in_to_read numero registri in INGRESSO da leggere
		3, // num_reg_in_to_write numero registri in USCITA da leggere
		0, // numero del registro in INGRESSO interessato alla operazione
		0 // numero del registro in USCITA interessato alla operazione
		
    ) != 0 ) {
      exit(-1);
    }
  }

  modbus_close(mb_otb);
  modbus_free(mb_otb);

  return 0;
}
