.SUFFIXES : .o.c

CC = gcc
# librerie e include per modbus
INCDIR = /home/reario/include
LIBDIR = /home/reario/lib

# librerie e include per postgres
LIBDB = /usr/local/pgsql/lib
DBINCDIR = /usr/include/postgresql/

objs = value.o operate.o setplctime.o
op = operate.o bit.o

all: value operate setplctime event ss readtime #enum secondi scatti myserver

value:  pgdb.o value.o
	$(CC) -Wall -L${LIBDB} -L${LIBDIR} -lmodbus -lpq $^ -o $@

operate: bit.o operate.o
	$(CC) -Wall -L${LIBDIR} -lmodbus -lncurses $^ -o $@

setplctime: setplctime.o
	$(CC) -Wall  -L${LIBDIR} -lmodbus $^ -o $@

readtime: readtime.o
	$(CC) -Wall  -L${LIBDIR} -lmodbus $^ -o $@

ss: pgdb.o ss.o
	$(CC) -Wall -L${LIBDB} -L${LIBDIR} -lmodbus -lpq $^ -o $@

event: event.o
	$(CC) -Wall -L${LIBDB} -L${LIBDIR} -lmodbus -lpq -lm $^ -o $@

event-test: event-test.o
	$(CC) -Wall -L${LIBDB} -L${LIBDIR} -lmodbus -lpq -lm $^ -o $@


# vengono costruiti fli object
.c.o: gh.h
	$(CC) -c -g -DDOINSERT3 -Wall -I${DBINCDIR} -I$(INCDIR)/modbus $< -o $@

# cancella i file non necessari e pulisce la directory, pronta per una compilazione pulita
clean :
	rm -f *~ *.o *.i *.s *.core readtime value operate scatti secondi setplctime myserver event enum




########################################################################################################################
# myserver: myserver.o
#       $(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -lmodbus -lpq $^ -o $@
#scatti: db.o scatti.o
#       $(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -L${DBLIBDIR} -lmodbus -lncurses -lmysqlclient $^ -o $@
#secondi: db.o secondi.o
#       $(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -L${DBLIBDIR} -lmodbus -lncurses -lmysqlclient $^ -o $@
#enum: enum.o
#       $(CC) -g -Wall -L${LIBDIR} -lgsl -lgslcblas -lm $^ -o $@
#SCRIPT_SH = value.sh scatti.sh secondi.sh ss.sh
########################################################################################################################
