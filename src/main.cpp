#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "AtariAudio/AtariAudio.h"

// same constant used in AtariAudio/AtariMachine.h
// yes, this will make maintaining a pain, but i can't access it 
#define SNDH_LOAD_ADDRESS 	0x10002

void usage() {
	fprintf(stderr,
		"usage: sndhdump [options] <sndh_file>\n"
		"options:\n"
		"	-s<frames>\tamount of frames to skip ahead before running the SNDH\n"
		"	-r\t\tprint raw registers only\n"
		"	-f<frames>\tamount of frames to run the SNDH for\n"
	);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		usage();
		return -1;
	}
	
	int mode = 0; // 0 - pretty dump, 1 - raw dump
	int fileC = 0; // file argument position
	int frames = 100000;
	int skipFrames = 0;
	
	for (int i=1; i<argc; ++i) {
		const char* p = argv[i];
		if (*p != '-') {
			fileC = i;
			break;
		}
		switch (p[1]) {
			case 's':
				skipFrames = atoi(p+2);
				break;
			case 'f':
				frames = atoi(p+2);
				break;
			case 'r':
				mode = 1;
				break;
			default:
				fprintf(stderr, "sndhdump: option not found\n");
				return -1;
		}
	}
	
	// ask user to make sure they actually want to spam their terminal
	if (isatty(fileno(stdout))) {
		int c;
		while (c != 'y' && c != 'Y' && c != 'n' && c != 'N') {
			fprintf(stderr, "sndhdump has not been redirected to a file, continue? (y/n): ");
			c = getchar();
		}
		if (c == 'n' || c == 'N') return -1;
	}
	
	SndhFile sndh;
	SndhFile::SubSongInfo info;
	
	FILE* f = fopen(argv[fileC], "rb");
	fseek(f, 0, SEEK_END);
	int sndhLen = ftell(f);
	rewind(f);
	void* sndhBuf = malloc(sndhLen);
	fread(sndhBuf, 1, sndhLen, f);
	fclose(f);
	
	sndh.Load(sndhBuf, sndhLen, 44100);
	
	free(sndhBuf);
	
	sndh.InitSubSong(1);
	sndh.GetSubsongInfo(1, info);
	
	printf("title:\t\t%s\n", info.musicName);
	printf("author:\t\t%s\n", info.musicAuthor);
	printf("year:\t\t%s\n", info.year);
	printf("tick rate:\t%dhz\n\n", info.playerTickRate);
	if (skipFrames) printf("skipping ahead %d frames from the beginning\n", skipFrames);
	printf("now running SNDH file for %d frames\n\n", frames);
	
	if (skipFrames)
		for (int i=0; i<skipFrames; i++) sndh.m_atariMachine.Jsr(SNDH_LOAD_ADDRESS + 8, 0);
	
	// this much nesting is not healthy i bet
	for (int f=0; f<frames; f++) {
		sndh.m_atariMachine.Jsr(SNDH_LOAD_ADDRESS + 8, 0);
		switch (mode) {
			case 0: {
				// pretty register dump
				// surely there must be a better way to do this
				// YM registers
				int periodA = sndh.m_atariMachine.m_Ym2149.m_regs[0] | (sndh.m_atariMachine.m_Ym2149.m_regs[1] << 8);
				int periodB = sndh.m_atariMachine.m_Ym2149.m_regs[2] | (sndh.m_atariMachine.m_Ym2149.m_regs[3] << 8);
				int periodC = sndh.m_atariMachine.m_Ym2149.m_regs[4] | (sndh.m_atariMachine.m_Ym2149.m_regs[5] << 8);
				int envPeriod = sndh.m_atariMachine.m_Ym2149.m_regs[11] | (sndh.m_atariMachine.m_Ym2149.m_regs[12] << 8);
				int noisePeriod = sndh.m_atariMachine.m_Ym2149.m_regs[6];
				printf("frame %08d\n", f);
				printf("\tfreq A: %04x, freq B: %04x, freq C: %04x\n", periodA, periodB, periodC);
				printf("\tnoise freq: %02x, envelope freq: %04x\n", noisePeriod, envPeriod);
				
				int mixerControl = sndh.m_atariMachine.m_Ym2149.m_regs[7];
				printf("\ttone  enabled: ");
				if (!(mixerControl & 1)) printf("A "); else printf("  ");
				if (!(mixerControl & 2)) printf("B "); else printf("  ");
				if (!(mixerControl & 4)) printf("C "); else printf("  ");
				printf("\n");
				printf("\tnoise enabled: ");
				if (!(mixerControl & 8)) printf("A "); else printf("  ");
				if (!(mixerControl & 16)) printf("B "); else printf("  ");
				if (!(mixerControl & 32)) printf("C "); else printf("  ");
				printf("\n");
				
				int volA = sndh.m_atariMachine.m_Ym2149.m_regs[8];
				int volB = sndh.m_atariMachine.m_Ym2149.m_regs[9];
				int volC = sndh.m_atariMachine.m_Ym2149.m_regs[10];
				int envShape = sndh.m_atariMachine.m_Ym2149.m_regs[13];
				printf("\tvolume A: %d ", volA & 0x0f);
				if (volA & 0x10) printf("(envelope enabled)");
				printf("\n");
				printf("\tvolume B: %d ", volB & 0x0f);
				if (volB & 0x10) printf("(envelope enabled)");
				printf("\n");
				printf("\tvolume C: %d ", volC & 0x0f);
				if (volC & 0x10) printf("(envelope enabled)");
				printf("\n");
				
				printf("\tenvelope shape: %02x ", envShape);
				if (!(envShape & 0x08)) {
					if (envShape & 0x04) printf("(\\___)");
					else printf("(/|__)");
				} else {
					switch (envShape & 0x07) {
						case 0: printf("(\\|\\|)"); break;
						case 1: printf("(\\___)"); break;
						case 2: printf("(\\/\\/)"); break;
						case 3: printf("(\\|¯¯)"); break;
						case 4: printf("(/|/|)"); break;
						case 5: printf("(/¯¯¯)"); break;
						case 6: printf("(/\\/\\)"); break;
						case 7: printf("(/|__)"); break;
					}
				}
				printf("\n");
				// MFP registers
				const int prescalers[] = {0,4,10,16,50,64,100,200};
				// TxCR - timer control registers, TxDR - timer data registers
				int mfpTACR = sndh.m_atariMachine.m_Mfp.m_regs[0x19];
				int mfpTBCR = sndh.m_atariMachine.m_Mfp.m_regs[0x1b];
				int mfpTCDCR = sndh.m_atariMachine.m_Mfp.m_regs[0x1d];
				
				int mfpTADR = sndh.m_atariMachine.m_Mfp.m_regs[0x1f];
				int mfpTBDR = sndh.m_atariMachine.m_Mfp.m_regs[0x21];
				int mfpTCDR = sndh.m_atariMachine.m_Mfp.m_regs[0x23];
				int mfpTDDR = sndh.m_atariMachine.m_Mfp.m_regs[0x25];
				
				printf("\ttimer A data: %02x, prescaler: %02x (1/%d)\n", mfpTADR, (mfpTACR & 7), prescalers[(mfpTACR & 7)]);
				printf("\ttimer B data: %02x, prescaler: %02x (1/%d)\n", mfpTBDR, (mfpTBCR & 7), prescalers[(mfpTBCR & 7)]);
				printf("\ttimer C data: %02x, prescaler: %02x (1/%d)\n", mfpTCDR, (mfpTCDCR & 0x70), prescalers[(mfpTCDCR & 0x70) >> 4]);
				printf("\ttimer D data: %02x, prescaler: %02x (1/%d)\n", mfpTDDR, (mfpTCDCR & 7), prescalers[(mfpTCDCR & 7)]);
				break;
			}
			default: {
				// raw register dump
				printf("-YM registers-- 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d -MFP- 19 1b 1f 21 1d 23 25\n");
				for (int f=0; f<frames; f++) {
					sndh.m_atariMachine.Jsr(SNDH_LOAD_ADDRESS + 8, 0);
					printf("frame %08d: ", f);
					// YM registers
					for (int i=0; i<14; i++) {
						printf("%02x ", sndh.m_atariMachine.m_Ym2149.m_regs[i]);
					}
					// MFP registers
					printf("-MFP- ");
					for (int i=0x19; i<=0x25; i+=2) {
						printf("%02x ", sndh.m_atariMachine.m_Mfp.m_regs[i]);
					}
					printf("\n");
				}
				break;
			}
		}
	}
	
	return 0;
}