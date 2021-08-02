DEFS = umps/const.h h/types10.h umps/base.h umps/uMPStypes.h h/listx.h Makefile

CFLAGS =  -Wall -c -O0
LDFLAGS =  -T /usr/share/uMPS/elf32ltsmip.h.umpscore.x /usr/lib/uMPS/crtso.o /usr/lib/uMPS/libumps.o
CC = mipsel-linux-gcc 
LD = mipsel-linux-ld

all: kernel.core.umps tape0.umps disk0.umps

tape0.umps: kernel.core.umps
	umps-mkdev -t $@ $<

disk0.umps:
	umps-mkdev -d $@

kernel.core.umps: kernel
	umps-elf2umps -k $<

kernel: pcb.o asl.o initial.o scheduler.o interrupts.o exceptions.o p2test.0.1.o 
	$(LD) $(LDFLAGS) pcb.o asl.o initial.o scheduler.o interrupts.o exceptions.o p2test.0.1.o -o $@

pcb.o: src/pcb.c $(DEFS) 
	$(CC) $(CFLAGS)  src/pcb.c

asl.o: src/asl.c $(DEFS) 
	$(CC) $(CFLAGS)  src/asl.c
	
scheduler.o: src/scheduler.c $(DEFS) e/scheduler.e
	$(CC) $(CFLAGS)  src/scheduler.c
	
initial.o: src/initial.c $(DEFS)
	$(CC) $(CFLAGS)  src/initial.c	
	
interrupts.o: src/interrupts.c $(DEFS)
	$(CC) $(CFLAGS)  src/interrupts.c	
	
exceptions.o: src/exceptions.c $(DEFS) 
	$(CC) $(CFLAGS)  src/exceptions.c	
	
p2test.0.1.o: src/p2test.0.1.c $(DEFS)
	$(CC) $(CFLAGS) -O0 src/p2test.0.1.c

clean:
	./.rm_script.sh
