PREFIX         = v810
LIBERIS        = $(HOME)/devel/liberis
V810GCC        = $(HOME)/devel/pcfx/bin/v810-gcc

ASFLAGS        = -a=$*.lst
# CFLAGS        += -I$(LIBERIS)/include/ -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -O2 -Wall -std=gnu99 -mv810 -msda=256 -mprolog-function
CFLAGS        += -I$(LIBERIS)/include/ -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -Wall -std=gnu99 -mv810 -msda=256 -mprolog-function
CPFLAGS       += -I$(LIBERIS)/include/ -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -O2 -Wall -std=gnu++11 -fno-rtti -fno-exceptions -mv810 -msda=256 -mprolog-function
LDFLAGS       += -T$(LIBERIS)/ldscripts/v810.x -L$(LIBERIS)/ -L$(V810GCC)/lib/ -L$(V810GCC)/$(PREFIX)/lib/ -L$(V810GCC)/lib/gcc/$(PREFIX)/4.7.4/ $(V810GCC)/$(PREFIX)/lib/crt0.o

LIBS           = -leris -lc -lsim -lgcc

hexview.cue: cdlink_hexview.txt hexview
	pcfx-cdlink cdlink_hexview.txt hexview

hexview.flashboot: hexview
	python3 mkflashboot.py hexview

hexview: hexview.o font.o readaddr.o
	v810-ld $(LDFLAGS) hexview.o font.o readaddr.o $(LIBS) --sort-common=descending -o hexview.linked -Map hexview.map
	v810-objcopy -O binary hexview.linked hexview

font.o: font.s
	v810-as $(ASFLAGS) font.s -o font.o

readaddr.o: readaddr.s
	v810-as $(ASFLAGS) readaddr.s -o readaddr.o

hexview.o: hexview.source
	v810-as $(ASFLAGS) hexview.source -o hexview.o

%.o: %.s
	v810-as $(ASFLAGS) $< -o $@

hexview.source: hexview.c
	v810-gcc $(CFLAGS) hexview.c -S -o hexview.source

%.elf: $(OBJECTS)
	v810-ld $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@ -Map $*.map

%.bin: %.elf
	v810-objcopy -O binary $< $@

lbas.h:
	bincat out.bin lbas.h $(BIN_TARGET) $(ADD_FILES)

clean:
	rm -rf hexview *.o *.source *.map *.lst *.linked lbas.h out.bin hexview.bin hexview.cue
