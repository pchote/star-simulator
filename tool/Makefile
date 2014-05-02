CC       = gcc
CFLAGS   = -g -Wall -Wno-unknown-pragmas -pedantic --std=c99
LFLAGS   =

# Statically link libgcc and libstdc++ to avoid needing extra dlls under windows
# Force ANSI-style printf formatting
ifeq ($(MSYSTEM),MINGW32)
    CFLAGS += -DWIN32 -D__USE_MINGW_ANSI_STDIO=1 -m32
    LFLAGS += -static-libgcc -m32
endif

starsimulator: tool.o serial.o
	$(CC) -o $@ tool.o serial.o $(LFLAGS)

clean:
	-rm serial.o tool.o starsimulator starsimulator.exe

%.o : %.c
	$(CC) -c $(CFLAGS) $<
