include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard *.c)
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_C=$(wildcard C/*.c)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
OBJECTS_C=$(SOURCES_C:.c=.o)

all: $(MODULE)

$(MODULE): $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS) $(OBJECTS_C)
	$(LD) $(LDFLAGS) -T sampleCodeModule.ld -o ../$(MODULE) $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_C)
%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@
%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf asm/*.o C/*.o *.o

.PHONY: all clean print