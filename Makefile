.SUFFIXES : .o.c

CC = gcc 

objs = value.o operate.o setplctime.o
op = operate.o bit.o
INCDIR = /home/reario/include
LIBDIR = /home/reario/lib
DBINCDIR = /usr/include/postgresql


SCRIPT_SH = value.sh scatti.sh secondi.sh ss.sh
all: value operate setplctime ss event enum #secondi scatti myserver 

value:  pgdb.o value.o
	$(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -lmodbus -lncurses  -lpq $^ -o $@

operate: bit.o operate.o
	$(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -lmodbus -lncurses $^ -o $@

setplctime: setplctime.o  
	$(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -lmodbus -lncurses $^ -o $@

ss: pgdb.o ss.o
	$(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -lmodbus -lncurses -lpq $^ -o $@

# myserver: myserver.o
# 	$(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -lmodbus -lpq $^ -o $@

#scatti: db.o scatti.o
#	$(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -L${DBLIBDIR} -lmodbus -lncurses -lmysqlclient $^ -o $@

#secondi: db.o secondi.o
#	$(CC) -Wall -I${INCDIR}/modbus -I${DBINCDIR} -L${LIBDIR} -L${DBLIBDIR} -lmodbus -lncurses -lmysqlclient $^ -o $@

event: event.o
	$(CC) -Wall -L${LIBDIR} -lmodbus -lpq $^ -o $@

enum: enum.o
	$(CC) -g -Wall -L${LIBDIR} -lgsl -lgslcblas -lm $^ -o $@

.c.o: gh.h
	$(CC) -c -g -DDOINSERT3 -Wall -I$(INCDIR)  -I${DBINCDIR} -I$(INCDIR)/modbus $< -o $@

clean :
	rm -f *~ *.o *.i *.s *.core value operate scatti secondi setplctime myserver event enum

