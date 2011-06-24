# Makefile per mips-linux

# Dichiarazione delle cartelle base
INCLUDE = ./include
PHASE1PATHE = ./phase1/e
PHASE2PATHE = ./phase2/e
PHASE1PATHSRC = ./phase1/src
PHASE2PATHSRC = ./phase2/src
LIBPATH = /usr/lib/uMPS
ELFPATH = /usr/include/uMPS
ELF32 = /usr/share/uMPS

# Dichiarazione dei comandi base
CFLAGS = -Wall -I $(INCLUDE) -I $(PHASE1PATHE) -I $(PHASE2PATHE) -I $(ELFPATH) -I $(ELF32) -c
LDFLAGS =  -T
CC = mipsel-linux-gcc
LD = mipsel-linux-ld

# Target principale
all: kernel.core.umps tape0.umps disk0.umps

tape0.umps: kernel.core.umps
	umps-mkdev -t $@ $<

disk0.umps: kernel.core.umps
	umps-mkdev -d $@

kernel.core.umps: kernel
	umps-elf2umps -k kernel

# Linking del kernel
kernel: phase1dir phase2dir $(LIBPATH)/crtso.o $(LIBPATH)/libumps.o
	$(LD) $(LDFLAGS) 	$(ELF32)/elf32ltsmip.h.umpscore.x \
				$(LIBPATH)/crtso.o \
				$(PHASE1PATHSRC)/asl.o \
				$(PHASE1PATHSRC)/pcb.o \
				$(PHASE2PATHSRC)/p2test.0.1.o \
				$(PHASE2PATHSRC)/initial.o \
				$(PHASE2PATHSRC)/scheduler.o \
				$(PHASE2PATHSRC)/exceptions.o \
				$(PHASE2PATHSRC)/interrupts.o \
				$(LIBPATH)/libumps.o \
				-o kernel

# Per testare phase1, inserire l'oggetto del p1test tra i file richieste per il linking del kernel

# Sorgenti di phase1
phase1dir:
	cd $(PHASE1PATHSRC) && make all

# Sorgenti di phase2
phase2dir:
	cd $(PHASE2PATHSRC) && make all

# Pulizia parziale dei file creati
clean:
	rm -f *.o kernel
	rm -f kernel.*.umps
	rm -f term*.umps printer*.umps tape0.umps disk0.umps

# Pulizia totale dei file creati
cleanall:
	rm -f *.o kernel
	rm -f kernel.*.umps
	rm -f term*.umps printer*.umps tape0.umps disk0.umps
	cd $(PHASE1PATHSRC) && make clean
	cd $(PHASE2PATHSRC) && make clean
	rm -rf ./doc

# Documentazione
doc:
	-doxygen
