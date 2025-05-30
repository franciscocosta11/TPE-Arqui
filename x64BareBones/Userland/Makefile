include Makefile.inc

USERLAND=userland.bin
SAMPLE_DATA=0001-sampleDataModule.bin
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_C=$(wildcard c/*.c)

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
OBJECTS_C=$(SOURCES_C:.c=.o)

all: sampleCodeModule sampleDataModule #$(USERLAND)

sampleCodeModule:
	cd SampleCodeModule; make

sampleDataModule:
	printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

clean:
	cd SampleCodeModule; make clean
	rm -rf *.bin


.PHONY: sampleCodeModule all clean
