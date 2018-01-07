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

#define NB_CONNECTION    15
#define WORD 16
#define DWORD 32
#define QWORD 64

/*
PLC
 */
#define I0_16 65 /* word che contiene lo stato degli input 0-16*/
#define I16_32 66 /* word che contiene lo stato degli input 16-32 */
#define Q0_32 67 /* word che contiene lo stato degli output 0-32*/

/*
OTB
 */
#define OTBDIN 74  /* word che contiene lo stato degli input 0-7 dell'OTB*/
#define OTBAIN1 75 /* word che contiene lo stato dell'input analogico 1 dell'OTB (BAR AUTOCLAVE) */
#define OTBAIN2 76 /* word che contiene lo stato dell'input analogico 3 dell'OTB (BAR POZZO) */
#define OTBDOUT 77 /* word che contiene lo stato degli output 0-7 dell'OTB */
#define OTBAOUT1 78 /* word che contiene lo stato dell'output analogico 1 dell'OTB */
#define OTBAOUT2 79 /* word che contiene lo stato dell'output analogico 2 dell'OTB */

#define CONN "hostaddr=192.168.1.103 user=reario dbname=reario_db"; // <-- DB postgres

modbus_t *ctx = NULL;
int server_socket = -1;
modbus_mapping_t *mb_mapping;

/* per ognuno degli eventi viene memorizzato il codice (massimo QWORD eventi, cioè 64). Ogni codice è un timestamp */
unsigned long long eventkeys[QWORD];

char *inputs_names[]={ 
  "AUTOCLAVE", // 0
  "POMPA_SOMMERSA", // 1
  "RIEMPIMENTO", // 2
  "LUCI_ESTERNE_SOTTO", // 3
  "CENTR_R8", // 4
  "LUCI_GARAGE_DA_4", // 5
  "LUCI_GARAGE_DA_2", // 6
  "LUCI_TAVERNA_1_di_2", // 7
  "LUCI_TAVERNA_2_di_2", // 8
  "INTERNET",            // 9
  "C9912",              // 10
  "LUCI_CUN_LUN",       // 11
  "LUCI_CUN_COR",       // 12
  "LUCI_STUDIO_SOTTO",  // 13
  "LUCI_ANDRONE_SCALE", // 14
  "GENERALE_AUTOCLAVE", // 15
  "LUCI_CANTINETTA",    // 16 
  "PLC_INPUT_17", // 17
  "PLC_INPUT_18", // 18
  "PLC_INPUT_19", // 19
  "PLC_INPUT_20", // 20
  "PLC_INPUT_21", // 21
  "PLC_INPUT_22", // 22
  "PLC_TM2_INPUT_0",  // 23
  "PLC_TM2_INPUT_1",  // 24
  "PLC_TM2_INPUT_2",  // 25
  "PLC_TM2_INPUT_3",  // 26
  "PLC_TM2_INPUT_4",  // 27
  "PLC_TM2_INPUT_5",  // 28
  "PLC_TM2_INPUT_6",  // 29
  "PLC_TM2_INPUT_7",  // 30
  "PLC_TM2_INPUT_8",  // 31
  "PLC_TM2_INPUT_9",  // 32
  "PLC_TM2_INPUT_10", // 33
  "PLC_TM2_INPUT_11", // 34
  "PLC_TM2_INPUT_12", // 35
  "PLC_TM2_INPUT_13", // 36
  "PLC_TM2_INPUT_14", // 37
  "PLC_TM2_INPUT_15", // 38
  "OTB_DIN_0", // 39
  "OTB_DIN_1", // 40
  "OTB_DIN_2", // 41
  "OTB_DIN_3", // 42
  "OTB_DIN_4", // 43
  "OTB_DIN_5", // 44
  "OTB_DIN_6", // 45
  "OTB_DIN_7", // 46
  "OTB_DIN_8", // 47
  "OTB_DIN_9", // 48
  "FARI_ESTERNI_SOTTO",// 49 (OTB)
  "FARI_ESTERNI",// 50  (OTB)
  "IN_51",  // 51
  "IN_52",  // 52
  "IN_53",  // 53
  "IN_54",  // 54
  "IN_55",  // 55
  "IN_56",  // 56
  "IN_57",  // 57
  "IN_58",  // 58
  "IN_59",  // 59
  "IN_60",  // 60
  "IN_61",  // 61
  "IN_62",  // 62
  "IN_63"   // 63

};

uint64_t place (uint64_t dest, uint16_t source, uint16_t pos) {

  /* places the string of bit represented by source in position pos (starting from left) into dest */
 
  uint64_t temp=0;

  temp=temp|source;
  temp=temp<<pos;
  dest=dest|temp;
  return dest;

}


void printbitssimple64(uint64_t n) {
  /*dato l'intero n stampa la rappresentazione binaria*/
  uint64_t i;
  int j;
  i = (uint64_t)1<<(sizeof(n) * 8 - 1); /* 2^n */
  for (j=63;j>=0;j--) {   
    printf(" %2i",j);
  }
  printf("\n");  

  while (i > 0) {
    if (n & i)
      printf(" %2i",1);
    else
      printf(" %2i",0);
    i >>= 1;
  }
  printf("\n");
}


void printbitssimple32(uint32_t n) {
  /*dato l'intero n stampa la rappresentazione binaria*/
  uint32_t i;
  int j;
  i = (uint32_t)1<<(sizeof(n) * 8 - 1); /* 2^n */
  for (j=31;j>=0;j--) {   
    printf(" %2i",j);
  }
  printf("\n");  

  while (i > 0) {
    if (n & i)
      printf("1");
    else
      printf("0");
    i >>= 1;
  }
  printf("\n");
}

void printbitssimple16(uint16_t n) {
  /*dato l'intero n stampa la rappresentazione binaria*/
  uint16_t i;
  int j;
  i = (uint16_t)1<<(sizeof(n) * 8 - 1); /* 2^n */

  for (j=15;j>=0;j--) {   
    printf(" %2i",j);
  }
  printf("\n");  
  while (i > 0) {
    if (n & i)
      printf("1");
    else
      printf("0");
    i >>= 1;
  }
  printf("\n");
}


uint16_t read_single_state(uint16_t reg, uint16_t q) {
  /*legge q-esomo bit di reg*/
  uint16_t i;
  i=(1<<q); /* 2^q */
  if (reg & i) {return 1;} else {return 0;};
}


#ifdef CUMULATIVE
/* definizione di variabili per calcolo statistico della tensione */
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

/*
-------------------  INSERT3 -----------------------
*/
int insert3 (char *input, uint64_t stato, uint16_t pos) {
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
    /* metto +pos perchè se l'aggiornamento avviene su due bit all'interno dello stesso secondo,
       la chiave sarebbela stessa per entrambi i bit in quanto il timestamp è in secondi*/
    eventkeys[pos]=time(NULL)+pos; 
    sprintf(query,"INSERT INTO events (data,input,orastart,orastop,ts) values ('%s','%s','%s',NULL,'%lld');",datestring,input,timestring,eventkeys[pos]);   
  } else {/*OFF*/
    printf("POS=%d - eventkeys[%d]=%lld\n",pos,pos,eventkeys[pos]);
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
      
      /* Run through the existing connections looking for data to be read */
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
	    
	      printf("New connection from %s:%d on socket %d\n",
	      inet_ntoa(clientaddr.sin_addr),
	      clientaddr.sin_port, 
	      newfd);
	    
	  }
	} else {
	//----------------------
	  modbus_set_socket(ctx, master_socket);
	  rc = modbus_receive(ctx, query);
	  //printf("A client is asking a new connection\n");	  
	  if (rc > 0) {
	    modbus_reply(ctx, query, rc, mb_mapping);		    
	    printf("client ha letto dal registro\n");
	  } else if (rc == -1) {
	    /*server is ended on connection closing or
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
	//----------------------
      }
    }
    
    return 0;
}
