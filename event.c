/*
  Questo programma scrive nel DB postgres sulla macchina 192.168.1.103 (vedi parametro  CONN) i valori degli eventi I/O digitali che si verificano.
  Esso inoltre acquisisce i valori di tensione, corrente e potenza nonchè i valori degli I/O digitali dell'OTB e dei valori pressione pozzo e autoclave. 
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <modbus.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "libpq-fe.h"
#include <time.h>
#include <math.h>

#define NB_CONNECTION    5
#define WORD 16
#define DWORD 32

#define I0_16 65 /* word che contiene lo stato degli input 0-16*/
#define I16_32 66 /* word che contiene lo stato degli input 16-32 */
#define Q0_32 67 /* word che contiene lo stato degli output 0-32*/

#define CONN "hostaddr=192.168.1.103 user=reario dbname=reario_db"; // <-- DB postgres

modbus_t *ctx = NULL;
int server_socket = -1;
modbus_mapping_t *mb_mapping;

unsigned long long eventkeys[DWORD];

char *inputs_names[]={
  "AUTOCLAVE",
  "POMPA_SOMMERSA",
  "RIEMPIMENTO",
  "LUCI_ESTERNE_SOTTO",
  "CENTR_R8",
  "LUCI_GARAGE_DA_4",
  "LUCI_GARAGE_DA_2",
  "LUCI_TAVERNA_1_di_2",
  "LUCI_TAVERNA_2_di_2",
  "INTERNET",
  "C9912",
  "LUCI_CUN_LUN",
  "LUCI_CUN_COR",
  "LUCI_STUDIO_SOTTO",
  "LUCI_ANDRONE_SCALE",
  "GENERALE_AUTOCLAVE",
  "LUCI_CANTINETTA"
};

/* definizione di variabili per calcolo statistico della tensione */


#ifdef CUMULATIVE
unsigned long long VN; /* numero dei campioni rilevati, da inizializzare con il valore presente nel DB tabella Vstat*/
float sumVxi; /* somma dei campioni, da inizializzare con il valore presente nl DB tabella Vstat*/
float sumVxi2; /* somma del quadrato dei campioni, da inizializzare con il valore presente nl DB tabella Vstat*/
 
int updateVstat(unsigned long long N,float sumxi,float sumxi2) {
  char query[512]; 
  /* funzione che aggiorna i parametri statistici per calcolare la deviazione standard della tensione */
#ifdef DOINSERT3
  PGresult   *res;
  const char conninfo[]=CONN;
  PGconn *conn;
  char errormsg[256];
  conn = PQconnectdb(conninfo);
#endif

  sprintf(query,"UPDATE Vstat SET vn=%llu,sumVxi=%f,sumVxi2=%f",N,sumxi,sumxi2);
  
#ifdef DOINSERT3
  
  if (PQstatus(conn) != CONNECTION_OK) 
    {
      sprintf(errormsg,"%s\n", PQerrorMessage(conn));
      PQfinish(conn);
      printf("%s\n",errormsg);  
      return (1);
    }
  res=PQexec(conn, query);
  
  if (PQresultStatus(res) != PGRES_COMMAND_OK) { 
    
    sprintf(errormsg,"%s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    //    printf("[%s] [%s] [%s]\n %s\n",datestring,timestring,input,errormsg);
    return (1);
  }
  else { 
    sprintf(errormsg,"DELETE statement succeeded");
    //printf("%s\n",errormsg);
  }
  PQclear(res);
  PQfinish(conn);
#endif

    return(0);
}  /* end functio vstat */
#endif /* CUMULATIVE */



int inittable() {

  char query[512]; 

  /* funzione chiamata allo start, pulisce gli eventi 
     che sono trovati in ON (hanno orastop=NULL), inoltre inizializza VN, sumVxi e sumVxi2
  */
#ifdef DOINSERT3
  PGresult   *res;
  const char conninfo[]=CONN;
  PGconn *conn;
  char errormsg[256];
  conn = PQconnectdb(conninfo);
#endif
  
  sprintf(query,"DELETE FROM events WHERE orastop ISNULL");

#ifdef CUMULATIVE
  PGresult   *resV;
  char queryV[512];   
  sprintf(queryV,"SELECT vn,sumvxi,sumvxi2 from Vstat");
#endif

#ifdef DOINSERT3
  if (PQstatus(conn) != CONNECTION_OK) 
    {
      sprintf(errormsg,"%s\n", PQerrorMessage(conn));
      PQfinish(conn);
      printf("%s\n",errormsg);  
      return (1);
    }
  res=PQexec(conn, query);

#ifdef CUMULATIVE
  resV=PQexec(conn, queryV);
#endif

  if ( (PQresultStatus(res) != PGRES_COMMAND_OK)

#ifdef CUMULATIVE
    || (PQresultStatus(resV) != PGRES_COMMAND_OK)
#endif
    )
    { 
    
    sprintf(errormsg,"%s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    //    printf("[%s] [%s] [%s]\n %s\n",datestring,timestring,input,errormsg);
    return (1);
  }
  else { 
    sprintf(errormsg,"DELETE statement succeeded");
#ifdef CUMULATIVE
    sprintf(errormsg,"FETCH statement succeeded");

    /* inizio a prelevare i dati statistici: VN, sumVxi e sumVxi2 */
    
    VN=atoll(PQgetvalue(resV,0,0)); // riga0, campo 0 (VN)
    sumVxi=atof(PQgetvalue(resV,0,1)); // riga0, campo 1 (sumVxi)
    sumVxi2=atof(PQgetvalue(resV,0,2)); // riga0, campo 2 (sumVxi2)
#endif
    //printf("%s\n",errormsg);
  }
#ifdef CUMULATIVE
  PQclear(resV);
#endif
  PQclear(res);
  PQfinish(conn);
#endif

    return(0);
}


int insert3 (char *input, int stato, int pos) {

  /*stato: 0=OFF, 1=ON*/
  
  time_t timer;

  char datestring[11];
  char timestring[9];
  char query[512]; 
  time(&timer);
  
#ifdef DOINSERT3 
  PGresult   *res;
  const char conninfo[]=CONN;
  PGconn *conn;
  char errormsg[256];
  conn = PQconnectdb(conninfo);
#endif 

  strftime(datestring, sizeof(datestring),"%Y-%m-%d",localtime(&timer));
  strftime(timestring, sizeof(timestring),"%H:%M:%S",localtime(&timer));
  
  if (stato==0) {/*ON*/
    /* mettere come chiave la concatenazione di data|orastart|input ? */
    eventkeys[pos]=time(NULL)+pos; /* metto +pos perchè se l'aggiornamento avviene su due bit all'interno dello stesso sec. la chiave sarebbela stessa per entrambi i bit in quanto il timestamp è in secondi*/

    sprintf(query,"INSERT INTO events (data,input,orastart,orastop,ts) values ('%s','%s','%s',NULL,'%lld');",datestring,input,timestring,eventkeys[pos]);   
  } else {/*OFF*/
    sprintf(query,"UPDATE events set orastop='%s' WHERE ts='%lld'",timestring,eventkeys[pos]);
  }

#ifdef DOINSERT3
  if (PQstatus(conn) != CONNECTION_OK) 
    {
      sprintf(errormsg,"%s\n", PQerrorMessage(conn));
      PQfinish(conn);
      printf("%s\n",errormsg);  
      return (1);
    }
  res=PQexec(conn, query);

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    sprintf(errormsg,"%s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    printf("[%s] [%s] [%s]\n %s\n",datestring,timestring,input,errormsg);
    return (1);
  }
  else { 
    sprintf(errormsg,"%s %s INSERT statement succeeded", datestring, timestring);
    // printf("%s\n",errormsg);
  }
  PQclear(res);
  PQfinish(conn);
#endif
    return(0);
}

/************************ PRINT  **************************/
void print_bit_status (modbus_mapping_t *mb_map,int n) {
  int i;
  //printf("%d\n",mb_map->nb_bits);  
  for (i=0; i<n; i++) {
    printf("%d %s\n",i,mb_map->tab_bits[i]==1?"ON":"OFF");
    // printf("**%d \n",mb_map->tab_bits[i]);
  }
}

void print_reg_status (modbus_mapping_t *mb_map,int n) {
  int i;
  for (i=0; i<n; i++) {
    printf("%d %d\n",i,mb_map->tab_registers[i]);
  }
}


/************************ COMPARE **************************/
int compare_bit_status(uint8_t *bits1,uint8_t *bits2) {
  return memcmp(bits1,bits2,WORD*sizeof(uint8_t) );
}

int compare_reg_status(uint16_t *regs1,uint16_t *regs2) {
  return memcmp(regs1,regs2,DWORD*sizeof(uint16_t) );
}
/*********************************************************/




uint16_t reverseBits16(uint16_t num)
{
  /* sizeof(num)=2 */
  uint16_t  NO_OF_BITS = sizeof(num) * 8;
  uint16_t reverse_num = 0;
  int i;
  for (i = 0; i < NO_OF_BITS; i++)
    {
      if((num & (1 << i)))
	reverse_num |= 1 << ((NO_OF_BITS - 1) - i);  
    }
  return reverse_num;
}		      

uint32_t reverseBits32(uint32_t num)
{
  /* sizeof(num)=4 */
  uint32_t  NO_OF_BITS = sizeof(num) * 8;
  uint32_t reverse_num = 0;
  int i;
  for (i = 0; i < NO_OF_BITS; i++)
    {
      if((num & (1 << i)))
	reverse_num |= 1 << ((NO_OF_BITS - 1) - i);  
    }
  return reverse_num;
}		      

static void close_sigint(int dummy)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);
    exit(dummy);
}
/*

 typedef struct {
    int nb_bits;
    int nb_input_bits;
    int nb_input_registers;
    int nb_registers;
    uint8_t *tab_bits;
    uint8_t *tab_input_bits;
    uint16_t *tab_input_registers;
    uint16_t *tab_registers;
} modbus_mapping_t

*/
/*
  Function codes 
#define _FC_READ_COILS                0x01
#define _FC_READ_DISCRETE_INPUTS      0x02
#define _FC_READ_HOLDING_REGISTERS    0x03
#define _FC_READ_INPUT_REGISTERS      0x04
#define _FC_WRITE_SINGLE_COIL         0x05 *
#define _FC_WRITE_SINGLE_REGISTER     0x06 *
#define _FC_READ_EXCEPTION_STATUS     0x07
#define _FC_WRITE_MULTIPLE_COILS      0x0F 15
#define _FC_WRITE_MULTIPLE_REGISTERS  0x10 16
#define _FC_REPORT_SLAVE_ID           0x11 17
#define _FC_MASK_WRITE_REGISTER       0x16 22
#define _FC_WRITE_AND_READ_REGISTERS  0x17 23
*/

int main(void)
{

    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    /* Maximum file descriptor number */
    int fdmax;


    uint8_t *prev_status_bit;
    uint16_t *prev_status_reg;

    uint16_t in1; /*prima parte dei 16 input, quelli da 0 a 15 : registro 65 sul plc master*/
    uint16_t in2; /*prima parte dei 16 input, quelli da 16 a 23 : registro 66 sul plc master*/
    uint32_t in,inprev,diff;  /* tutti gli input dentro una DWORD (32 bit) */
    float V,I,P,Vpre,Ipre,Ppre; /* Volt Ampere e Watt */




#ifdef CUMULATIVE
    float Vsigma; /* scarto quadratico medio della tensione */
#endif
    inprev=0;
    Vpre=0;
    Ipre=0;
    Ppre=0;


    ctx = modbus_new_tcp("NULL",502);
    //modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(500,500,500,500);

    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    /* metto a zero gli array degli stati */
    prev_status_bit = (uint8_t *) malloc(WORD*sizeof(uint8_t));
    memset(prev_status_bit, 0, WORD*sizeof(uint8_t));

    prev_status_reg = (uint16_t *) malloc(DWORD*sizeof(uint16_t));
    memset(prev_status_reg, 0, DWORD*sizeof(uint16_t));


    if (inittable()!=0) {
      printf("init table failed\n");
      exit(-1);
    } 
    
    for (;;) {
      fflush(NULL);
      rdset = refset;
      if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
	perror("Server select() failure.");
	close_sigint(1);
      }
      
      /* Run through the existing connections looking for data to be
       * read */
      for (master_socket = 0; master_socket <= fdmax; master_socket++) {
	
	if (!FD_ISSET(master_socket, &rdset)) {
	  continue;
	}
	
	if (master_socket == server_socket) {
	  /* A client is asking a new connection */

	  socklen_t addrlen;
	  struct sockaddr_in clientaddr;
	  int newfd;
	  
	  /* Handle new connections */
	  addrlen = sizeof(clientaddr);
	  memset(&clientaddr, 0, sizeof(clientaddr));
	  newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
	  if (newfd == -1) {
	    perror("Server accept() error");
	  } else {
	    FD_SET(newfd, &refset);
	    
	    if (newfd > fdmax) {
	      /* Keep track of the maximum */
	      fdmax = newfd;
	    }  
	    /*
	      printf("New connection from %s:%d on socket %d\n",
	      inet_ntoa(clientaddr.sin_addr),
	      clientaddr.sin_port, 
	      newfd);*/
	    
                }
	} else {
	  modbus_set_socket(ctx, master_socket);
	  rc = modbus_receive(ctx, query);
	  //	  printf("A client is asking a new connection\n");	  
	  if (rc > 0) {
		  modbus_reply(ctx, query, rc, mb_mapping);		    
		  /***********************************************************************************/
		  /* a questo punto il PLC ha scritto tutti i registri: 65 66 67 (I/O), 68,69 (I) 70,71 (V) 72,73 (P) */  
		  int x,offset = modbus_get_header_length(ctx);
		  /***** Estraggo il codice richiesta *****/
		  uint16_t address = (query[offset + 1]<< 8) + query[offset + 2];		  

		  switch (query[offset]) {
		  case 0x05: {/* il PLC sta chiedendo di scrivere BITS */		      
		    printf("Coil Registro %d-->%s [0x%02X]\n",address,(mb_mapping->tab_bits[address])?"ON":"OFF",query[offset]);     
		  }
		    break;
		  case 0x10:
		  case 0x06: {/* il PLC sta chiedendo di scrivere N registri  */

		    /*-------------------I/O--------------------------------------*/		  
		    in1=mb_mapping->tab_registers[I0_16];
		    in2=mb_mapping->tab_registers[I16_32];
		    
		    in1=reverseBits16(in1);
		    in2=reverseBits16(in2);
		    
		    in=(in1<<16)+in2;
		    in=reverseBits32(in);
		    
		    diff= in^inprev; /* xor: ogni 1 in diff significa che l'input è cambiato */
		    
		    if (diff) {		    

		      for (x=0;x < DWORD; x++) {
			if (diff & (1<<x)) { /* ho trovato 1 nella posizione x-esima di diff*/
			  /*vado ad analizzare se l'1 trovato è relativo ad una transizione 1->0 o 0->1 
			    diff contiene 1 se lo stato del bit e' cambiato. Che transizione è avvenuta? da on a off o da off a on?
			    se il bit x-esimo di *in* (vettore attuale degli ingressi) è 1 allora c'è stata la transizione da off a on.
			    se il bi x-esimo è di *in* è 0 allora c'è stata la transizione da on a off 
			  */
			  if (insert3(inputs_names[x],(inprev & (1<<x)),x)==1) {
			    printf("db error\n");
			  }
			  // printf("%s\t%s\n",inputs_names[x],(inprev & (1<<x))?"OFF":"ON");			  
			}
		      }
		      inprev=in;
		    } /* if (diff) */

		    /*-----------------------------------------------------------*/
		    /*----------------------- V,A,P -----------------------------*/
		    /*-----------------------------------------------------------*/
		    /*
		      68,69 - IL e IH corrente (reg 7 e 8 del plc)
		      70,71 - VL e VH tensione (reg 21 e 22 del plc)
		      72,73 - PL e PH potenza  (reg 29 e 30 del plc)
		     */
		    
		    
		    V=(float)((mb_mapping->tab_registers[70]<<16)+mb_mapping->tab_registers[71])/1000;
		    I=(float)((mb_mapping->tab_registers[68]<<16)+mb_mapping->tab_registers[69])/1000;
		    P=(float)((mb_mapping->tab_registers[72]<<16)+mb_mapping->tab_registers[73])/100;
		    		    
		     	
			if (Vpre != V) {
#ifdef CUMULATIVE
			  VN=VN+1;
			  sumVxi=sumVxi+V;
			  sumVxi2=sumVxi2+V*V; //pow(V,2);
			  updateVstat(VN,sumVxi,sumVxi2);
#endif
			  printf("-- V = %3.3f\n-- A = %3.3f\n-- P = %3.3f\n\n",V,I,P*1000);
			}
		     

		    Vpre=V;
		    Ipre=I;
		    Ppre=P; /*li salvo per il prossimo ciclo*/		    
		    
		  } /* case 0x06 */
		    break;
		  } /* switch */
		  
		  
		  /***********************************************************************************/
	  		  
		  
                } else if (rc == -1) {
		  /* This example server in ended on connection closing or
		   * any errors. */
		  //		   printf("Connection closed on socket %d\n", master_socket);
		  close(master_socket);
		  
		  /* Remove from reference set */
		  FD_CLR(master_socket, &refset);
		  
		  if (master_socket == fdmax) {
		    fdmax--;
		  }
                }
            }
        }
    }
    
    return 0;
}
