include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(shell find . -name "*.c")
ASM_SOURCES=$(shell find . -name "*.asm")
HEADERS=$(shell find . -name "*.h")
ASM_OBJECTS=$(ASM_SOURCES:.asm=.o)

all: $(MODULE)

$(MODULE): $(SOURCES) $(ASM_OBJECTS)
	$(GCC) $(GCCFLAGS) -T main.ld $(SOURCES) $(ASM_OBJECTS) -o ../$(MODULE)

%.o: %.asm
	nasm -felf64 $< -o $@

clean:
	rm -rf *.o

.PHONY: all clean print
