# Makefile di phase1

# Dichiarazione delle cartelle base
INCLUDE = ../../include
PHASE1PATHE = ../e
ELFPATH = /usr/include/uMPS
ELF32 = /usr/share/uMPS

# Dichiarazione dei comandi base
CFLAGS = -Wall -I $(INCLUDE) -I $(PHASE1PATHE) -I $(ELFPATH) -I $(ELF32) -c
CC = mipsel-linux-gcc

# Target principale
all: pcb.o asl.o
# Per testare phase1, commentare la riga precedente e decommentare le sottostanti
#all: pcb.o asl.o p1test.0.1.2.o

pcb.o: pcb.c
	$(CC) $(CFLAGS) pcb.c

asl.o: asl.c
	$(CC) $(CFLAGS) asl.c

#p1test.0.1.2.o: p1test.0.1.2.c
#	$(CC) $(CFLAGS) p1test.0.1.2.c

# Pulizia dei file oggetto
clean:
	rm -f *.o
