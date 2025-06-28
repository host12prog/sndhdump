CC = gcc
# for some reason, if CC is g++, the linker will throw a hissy fit
CFLAGS = -O2 -lstdc++ -lm -Isrc/AtariAudio -Isrc/AtariAudio/external
# we need -lm, or else the linker on Linux will complain
CFILES = src/main.cpp \
		src/AtariAudio/*.cpp \
		src/AtariAudio/external/ice_24.c \
		src/AtariAudio/external/Musashi/m68kcpu.c \
		src/AtariAudio/external/Musashi/m68kops.c
ifeq ($(OS),Windows_NT)
BIN_OUT = sndhdump.exe
else
BIN_OUT = sndhdump
endif
sndhdump: $(CFILES)
	$(CC) $(CFILES) $(CFLAGS) -o $(BIN_OUT)