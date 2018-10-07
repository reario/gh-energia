/* OPERATE */
/*

syntax: operate [on|off|read|flip] out

out: Q[0|Q1|Q2|Q3|4|5|6|7|8|9|10]

operate on <i> {turn Qi on}
operate off <i> {set Qi off}
operate read <i> {read Qi}
operate flip <i> {inverte Qi}
if command is INVERT and Qi>10 then all Qi will be flipped
if command is ON and Qi>10 then all Qi will be set
if command is OFF and Qi>10 then all Qi will be reset

*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <modbus.h>
#include <arpa/inet.h>
#include "gh.h"
#include <ncurses.h>
#include <math.h>

int row,col;

/*
#define OTB
*/

int pulsante(modbus_t *m,int bobina) {
  attron(COLOR_PAIR(1));mvprintw(2,col-4,"W");refresh();
  if ( modbus_write_bit(m,bobina,TRUE) != 1 ) {
    mvprintw(14,10,"ERRORE DI SCRITTURA:PULSANTE ON");
    refresh();
    return -1;
  }
  attroff(COLOR_PAIR(1));mvprintw(2,col-4,"W");refresh();
  sleep(1);
  attron(COLOR_PAIR(1));mvprintw(2,col-4,"W");refresh();
  if ( modbus_write_bit(m,bobina,FALSE) != 1 ) {
    mvprintw(14,10,"ERRORE DI SCRITTURA:PULSANTE OFF");
    refresh();
    return -1;
  }
  attroff(COLOR_PAIR(1));mvprintw(2,col-4,"W");refresh();
  return 0;
}

int interruttore(modbus_t *m, int bit, uint16_t reg) {
  /* set il bit iesimo di reg */
  reg=reg^(1<<bit);
  
  attron(COLOR_PAIR(1));mvprintw(2,col-4,"W");refresh();
  if ( modbus_write_register(m,100,reg) != 1 ) {
    mvprintw(14,10,"ERRORE DI SCRITTURA:INTERRUTTORE ON");
    refresh();
    return -1;
  }
  attroff(COLOR_PAIR(1));mvprintw(2,col-4,"W");refresh();
  return 0;
}

int main (int argc, char ** argv) {

  modbus_t *mb;
#ifdef OTB
  modbus_t *mb_otb;
#endif
  struct hostent *hp;
  int i,ch;
  int cont=1;


  uint16_t tab_reg[100]; /* vengono allocati 100  spazi anche se se ne utilizzano meno */
#ifdef OTB
  uint16_t otb_in[10];
  uint16_t otb_out[10];
#endif
  int nregs=69; /* 70 registri */
  int addr=7; /* offset */


  /* 
  DISTRIBUZIONE ORARIA SCATTI AUTOCLAVE E POZZO
  uint16_t tab_scatti[50];
  int nregs_scatti=51;
  int addr_scatti=100;
 
  DISTRIBUZIONE ORARIA SECONDI AUTOCLAVE E POZZO
  uint16_t tab_sec[50];
  int nregs_sec=51;
  int addr_sec=151;
  */

  int PATTIVAL, PATTIVAH,I1L, I1H, VINH, VINL,kWhH,kWhL;;
  float PATTIVA, VOLT, AMPERE, kWh;;
 
  int ERRORI=0;

  /* inizializzo ncurses */
  WINDOW *energia;
  initscr();
  start_color();
  init_pair(1,COLOR_BLUE,COLOR_YELLOW);
  init_pair(2,COLOR_WHITE,COLOR_BLUE);
  init_pair(3,COLOR_RED,COLOR_WHITE);
  noecho();
  cbreak();
  keypad(stdscr,TRUE);
  nodelay(stdscr,TRUE);
  while (cont) {
    curs_set(0);
    getmaxyx(stdscr,row,col);
    
    /* name resolution and connection to modbus device  */
    hp=gethostbyname(HOST);
    if (!hp) {
      printf("errore gethostbyname %d\n",h_errno);
      exit(1);
    }

    mb = modbus_new_tcp("192.168.1.157",PORT);
    //    mb = modbus_new_tcp( (char*)inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0])), PORT);
    //    mb = modbus_new_tcp( "192.168.1.157", PORT);

#ifdef OTB
    mb_otb = modbus_new_tcp("192.168.1.11",PORT);
#endif

    //    if (modbus_connect(mb) == -1) {printf("errore di connessione al PLC\n");exit(1);}
    //    if (modbus_connect(mb_otb) == -1) {printf("errore di connessione a OTB\n");exit(1);}

/*
#ifdef OTB
    if (modbus_connect(mb_otb) == -1) {
      printf("ERRORE non riesco a connettermi con OTB\n");
      fprintf(stderr,"connection failed--> %s\n",modbus_strerror(errno));
      exit(1);
    }
#endif

    if (modbus_connect(mb) == -1) {
      printf("ERRORE non riesco a connettermi con PLC\n"); 
      fprintf(stderr,"connection failed--> %s\n",modbus_strerror(errno));
      exit(1);
      //      exit(1);
    }
    */

    attron(COLOR_PAIR(1));mvprintw(0,col-4,"C");refresh();
    /* faccio la connessione */
    if ( (modbus_connect(mb) == -1) 

#ifdef OTB	 
	 || (modbus_connect(mb_otb) == -1) 
#endif

    ) 

      {

      printf("ERRORE non riesco a connettermi con il PLC\n");
      attroff(COLOR_PAIR(1));
      attron(COLOR_PAIR(3));
      mvprintw(0,col-4,"C");
      attroff(COLOR_PAIR(3));refresh();
      ERRORI=ERRORI+1;
      mvprintw(0,col-3,"%d",ERRORI);
      refresh();
      exit(1);
    } else {
      attroff(COLOR_PAIR(1));mvprintw(0,col-4,"C");refresh();
      attron(COLOR_PAIR(1));mvprintw(1,col-4,"R");refresh();
      
      /* leggo stato degli ingressi e i dati del PM9*/
      /* || (modbus_read_registers(mb, 75, 1, tab_reg+68) < 0) */
      if ( (modbus_read_registers(mb, addr, nregs, tab_reg) < 0) || 
	   (modbus_read_registers(mb, 507, 2, tab_reg+69) < 0) 
#ifdef OTB
           ||
	   (modbus_read_registers(mb_otb, 0, 3, otb_in) < 0) ||
	   (modbus_read_registers(mb_otb, 100, 3, otb_out) < 0)
#endif

      ) 
	{
	attroff(COLOR_PAIR(1));
	attron(COLOR_PAIR(3));
	mvprintw(1,col-4,"R");

	attroff(COLOR_PAIR(3));refresh();
	ERRORI=ERRORI+1;
	mvprintw(1,col-3,"%d",ERRORI);
	mvprintw(2,col-23,"%s\n",modbus_strerror(errno));
	refresh();
      } else { /* non ci sono stati errori di comunicazione e di lettura */
	attroff(COLOR_PAIR(1));mvprintw(1,col-4,"R");refresh();
	/******************/
	/* calcolo V,A,kW */
	VINL=tab_reg[22-7];
	VINH=tab_reg[21-7];
	I1L=tab_reg[8-7];
	I1H=tab_reg[7-7];
	PATTIVAL=tab_reg[30-7];
	PATTIVAH=tab_reg[29-7];
	kWhL=tab_reg[46-7];
	kWhH=tab_reg[45-7];
	VOLT= (float)(VINL+(VINH<<16))/1000 ;  
	AMPERE=(float)(I1L+(I1H<<16))/1000;
	PATTIVA=fabsf((float)(PATTIVAL+(PATTIVAH<<16))/100 ); /* valore assoluto in quanto viene potenza <0 (capacitiva) */      
	kWh=(float)(kWhL+(kWhH<<16));

	/******************************************************************************/
	/* STAMPO I VALORI DI V,A,kW in una nuova finestra*/
	energia=newwin(6,19,9,26);
	box(energia,0,0);
	wrefresh(energia);
	wattron(energia,A_BOLD);
	mvwprintw(energia,1,2,"VOLT = %3.2f",VOLT);
	mvwprintw(energia,2,2,"AMPERE = %3.2f",AMPERE);
	mvwprintw(energia,3,2,"kW = %1.2f",PATTIVA);
	wattroff(energia,A_BOLD);
	wrefresh(energia);
	/******************************************************************************/
	/* STAMPO LO STATO DEGLI INPUT DEL PLC*/
	i=0;
	while (i<=NUMINPUT-1) {
	  
	    /* stampo i tag per le voci dei menu*/
	    if ( (input[i].tag != ' ') && (input[i].tag != ' ') ) { /* stampo in bold i tag del menu */
	      attron(A_BOLD);
	      mvprintw( 0+((i<=12)?i:i-13),0+( (i<=12)?1:26),"%c-",input[i].tag);
	      attroff(A_BOLD);
	    }
	    if (read_single_state(tab_reg[(i<=15)?IN0:IN1],i-((i<=15)?0:16) ) ) {/* legge bit iesimo da IN0 o IN1 */
	      attron(COLOR_PAIR(1));
	      mvprintw( 0+((i<=12)?i:i-13),3+( (i<=12)?0:25),"%s",input[i].name);
	      attroff(COLOR_PAIR(1));
	    } else {
	      attron(COLOR_PAIR(0));
	      mvprintw( 0+((i<=12)?i:i-13),3+( (i<=12)?0:25),"%s",input[i].name);
	      attroff(COLOR_PAIR(0));
	    }
	  
	  i++;
	}

	/* STAMPO LO STATO DEGLI INPUT DEL OTB*/

	



	/******************************************************************************/
	/* Scrivo il numero di avvii pozzo e autoclave */
	mvprintw(0,13,"[%i/%i][%i]",tab_reg[NAU],tab_reg[SHA],tab_reg[SEGA]); /*Secondi Giornalieri Autoclave*/
	mvprintw(1,13,"[%i/%i][%i]",tab_reg[NPO],tab_reg[SHP],tab_reg[SEGP]); /*Secondi Giornalieri Pozzo*/
	
	mvprintw(15,25,"[bar = %1.2f]",(float)tab_reg[70]*0.00244200); /* bar registro 507  del PLC   messo nella posizione 70 del tab_reg */
	mvprintw(16,25,"[kWh = %4.0f]",(float)kWh);

	attron(A_BOLD);
	/* serratura */ 
	mvprintw(4,26,"O-");
	attroff(A_BOLD);
	mvprintw(4,28,"Serratura");
	
	
	/* apertura totale cancello */
	attron(A_BOLD);
	mvprintw(5,26,"N-");
	attroff(A_BOLD);
	mvprintw(5,28,"totale cancello");

	attron(A_BOLD);
	/* apertura parziale cancello */
	mvprintw(6,26,"P-");
	attroff(A_BOLD);
	mvprintw(6,28,"parziale cancello");

#ifdef OTB
	/* Fari LED esterni SOPRA */
	attron(A_BOLD);
	mvprintw(7,26,"R-");
	attroff(A_BOLD);
	if (otb_in[0] & (1<<FARI_ESTERNI_IN_SOPRA)) { // bit num 0
	  attron(COLOR_PAIR(1));
	  mvprintw(7,28,"fari esterni SOPRA");
	  attroff(COLOR_PAIR(1));
	} else {
	  attron(COLOR_PAIR(0));
	  mvprintw(7,28,"fari esterni SOPRA");
	  attroff(COLOR_PAIR(0));
	}

	/* Fari LED esterni SOTTO */
	attron(A_BOLD);
	mvprintw(8,26,"S-");
	attroff(A_BOLD);
	if (otb_in[0] & (1<<FARI_ESTERNI_IN_SOTTO)) { // bit num 1
	  attron(COLOR_PAIR(1));
	  mvprintw(8,28,"fari esterni SOTTO");
	  attroff(COLOR_PAIR(1));
	} else {
	  attron(COLOR_PAIR(0));
	  mvprintw(8,28,"fari esterni SOTTO");
	  attroff(COLOR_PAIR(0));
	}
#endif

	refresh();
	ch=getch();
	switch (ch) {
	case 'c':
	  if ( pulsante(mb,LUCI_ESTERNE_SOTTO) != 0 ) {
	    cont=0; }
	  break;
	case 'd':
	  if (pulsante(mb,LUCI_CUN_LUN) !=0 ) {
	    cont=0; }
	  break;
	case 'e':
	  if ( pulsante(mb,LUCI_CUN_COR)!=0) {
	    cont=0; }
	  break;
	case 'f':
	  if ( pulsante(mb,LUCI_TAVERNA)!=0) {
	    cont=0; }
	  break;
	case 'g':
	  if (pulsante(mb,LUCI_GARAGE)!=0) {
	    cont=0;}
	  break;
	case 'h':
	  if ( pulsante(mb,LUCI_ANDRONE_SCALE)!=0) {
	    cont=0; }
	  break;
	case 'i':
	  if ( pulsante(mb,LUCI_STUDIO_SOTTO)!=0) {
	    cont=0; }
	  break;
	case 'm':
	  if ( pulsante(mb,LUCI_CANTINETTA) !=0) {
	    cont=0;  }
	  break;
	case 'o':
	  if ( pulsante(mb,SERRATURA_PORTONE) !=0) {
	    cont=0;  }
	  break;
	case 'n':
	  if ( pulsante(mb,APERTURA_TOTALE) !=0) {
	    cont=0;  }
	  break;
	case 'p':
	  if ( pulsante(mb,APERTURA_PARZIALE) !=0) {
	    cont=0;  }
	  break;

#ifdef OTB
	case 'r':
	  if ( interruttore(mb_otb,FARI_ESTERNI_SOPRA,otb_out[0]) !=0) {
	    cont=0;  }
	  break;
	case 's':
	  if ( interruttore(mb_otb,FARI_ESTERNI_SOTTO,otb_out[0]) !=0) {
	    cont=0;  }
	  break;
#endif
     	case 'q':
	  cont=0;
	  break;
	}
      } /* else interno che wrappa il read_register */

    } /* else esterno che wrappa il connect*/

      
    sleep(1);

    modbus_close(mb);
    modbus_free(mb);
#ifdef OTB
    modbus_close(mb_otb);
    modbus_free(mb_otb);
#endif
    refresh();
    delwin(energia);
    endwin();

      
  } /*while*/
  return 0;
}
