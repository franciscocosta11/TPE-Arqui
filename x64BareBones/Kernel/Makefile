include Makefile.inc

KERNEL = kernel.bin

# Fuentes
SOURCES         = $(wildcard *.c)
SOURCES_ASM     = $(wildcard asm/*.asm)
SOURCES_C       = $(wildcard C/*.c)
SOURCES_DRIVERS = $(wildcard C/drivers/*.c)

# Objetos
OBJECTS         = $(SOURCES:.c=.o)
OBJECTS_ASM     = $(SOURCES_ASM:.asm=.o)
OBJECTS_C       = $(SOURCES_C:.c=.o)
OBJECTS_DRIVERS = $(SOURCES_DRIVERS:.c=.o)

# Loader
LOADERSRC    = loader.asm
LOADEROBJECT = $(LOADERSRC:.asm=.o)

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_DRIVERS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) \
	  $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_DRIVERS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf \
	  $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_DRIVERS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o C/*.o C/drivers/*.o *.o *.bin

.PHONY: all clean
