EE_BIN = ../data/PLUGINS/cleo.elf
EE_OBJS = main.o ../includes/pcsx2/memalloc.o ../includes/pcsx2/log.o utils.o text.o libres.o armhook.o touch.o ui.o pattern.o core.o mutex.o strutils.o memutils.o psplang.o 

CFLAGS = -O0 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -fpack-struct=16
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BASE_ADDRESS = 0x05000000
EE_LINKFILE = linkfile
EE_LIBS += -l:libstdc++.a -l:libm.a
EE_LDFLAGS = -Wl,--entry=init -Wl,--wrap,malloc -Wl,--wrap,calloc -Wl,--wrap,realloc -Wl,--wrap,free -Wl,--no-relax -Wl,-Map,../data/PLUGINS/cleo.map -Wl,'--defsym=BASE_ADDRESS=$(BASE_ADDRESS)'

all: clean main-build

main-build: $(EE_BIN)

clean:
	rm -f $(EE_OBJS) $(EE_BIN)

PS2SDK = ../external/ps2sdk/ps2sdk
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
