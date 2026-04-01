#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define TESTNUM 0x3c00

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

char calc_sd_crc7(char *buf) {
	int crc, crc_prev;
	int i,j;
	crc = buf[1];
	for(i=2; i<7; i++) {
		for(j=7; j>=0; j--) {
			crc <<= 1;
			crc_prev = crc;
			if (i<6) crc |= (buf[i]>>j) & 1;
			if (crc & 0x80) { crc ^= 0x89; }	// Generator
		}
	}
	return crc_prev | 1;
}


int cmd_dispatch(char *outbuf, char *inbuf, int len) {
	static volatile unsigned int* spi_mosi = (unsigned int*)0xc000f208;
	static volatile unsigned int* spi_miso = (unsigned int*)0xc000f20c;

	// reset miso fifo
	*spi_miso = 0x400;
	unsigned int rdata = 0x200;
	char data;

	if(len < 8) {

		// wiret mosi fiio
		for (int i = 0; i < len; i++) {
			*spi_mosi = outbuf[i];
			printf(" %02x",outbuf[i]);
		}
		// read miso fiio
		for (int i = 0; i < len; i++) {
			rdata = 0x200;
			while ((rdata & 0x200) == 0x200) {
				rdata = *spi_miso;
				printf(" %08x",rdata);
			}
			data = (char)(rdata & 0xff);
			inbuf[i] = data;
			printf(" %02x",data);
		}
	}
	else {
		// wiret mosi fiio only 4 data
		for (int i = 0; i < 4; i++) {
			*spi_mosi = outbuf[i];
			printf(" %02x",outbuf[i]);
		}
		// read and wirte both data
		for (int i = 4; i < len; i++) {
			*spi_mosi = outbuf[i];
			printf(" %02x",outbuf[i]);
			rdata = 0x200;
			while ((rdata & 0x200) == 0x200) {
				rdata = *spi_miso;
				printf(" %08x",rdata);
			}
			data = (char)(rdata & 0xff);
			inbuf[i-4] = data;
			printf(" %02x",data);
		}
		// read miso fiio
		for (int i = 0; i < 4; i++) {
			rdata = 0x200;
			while ((rdata & 0x200) == 0x200) {
				rdata = *spi_miso;
				printf(" %08x",rdata);
			}
			data = (char)(rdata & 0xff);
			inbuf[i+len-4] = data;
			printf(" %02x",data);
		}
	}
	printf("\n");
	for (int i = 0; i < len; i++) {
		printf(" %02x",inbuf[i]);
	}
	printf("\n");
	return len;
}

int spi_init() {
	static volatile unsigned int* spi_mode = (unsigned int*)0xc000f200;
	static volatile unsigned int* spi_sckdiv = (unsigned int*)0xc000f204;

	// initialization
	// sck : 250kHz
	printf("SPI register initialzation");
	*spi_sckdiv = 200;
	// CPOL=0 CPHA=0
	// SPI enable, big endian
	//*spi_mode = 0x9;
	*spi_mode = 0x00220009;

	// wait over 100 cycles
	//*spi_mosi = 0xff;
	for (int i = 0; i < 1000; i++) {
		printf("w");
	}
	return 0;
}


int main() {
	static volatile unsigned int* spi_mode = (unsigned int*)0xc000f200;
	static volatile unsigned int* spi_sckdiv = (unsigned int*)0xc000f204;
	static volatile unsigned int* spi_mosi = (unsigned int*)0xc000f208;
	static volatile unsigned int* spi_miso = (unsigned int*)0xc000f20c;

	char cmd0[8] = { 0xff, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff };
	char cmd1[8] = { 0xff, 0x41, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff };
	char cmd8[14] = { 0xff, 0x48, 0x00, 0x00, 0x01, 0xaa, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	char cmd9[8] = { 0xff, 0x49, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff };
	char cmd55[8] = { 0xff, 0x77, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff };
	char acmd41[8] = { 0xff, 0x69, 0x40, 0x00, 0x00, 0x00, 0x01, 0xff };
	char cmd5541[16] = { 0xff, 0x77, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x69, 0x40, 0xff, 0x80, 0x00, 0x01, 0xff };
	char dummy[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	char inbuf[64];
	int flg;

	char tmp = calc_sd_crc7(cmd0);
	cmd0[6] = tmp;

	printf("SPI init\n");
	spi_init();

//for(int k = 0; k < 2; k++) {
	// CMD 0
	printf("\nSPI SEND CMD0\n");
	flg = 0;

	while(flg == 0) {
		cmd_dispatch(cmd0, inbuf, 8);
		flg = (inbuf[7] == 0x01) ? 1 : 0;
	}
	wait();

	// CMD 8
	printf("\nSPI SEND CMD8\n");
	flg = 0;

	cmd_dispatch(cmd8, inbuf, 14);
	fflush(stdout);
	wait();

	// CMD 55  ACMD41
	printf("\nSPI SEND CMD55 ACMD41\n");
	flg = 0;

	while(flg == 0) {
		cmd_dispatch(cmd5541, inbuf, 16);
		flg = ((inbuf[8] == 0x01)&(inbuf[15] == 0x00)) ? 1 : 0;
		//flg = ((inbuf[8] == 0x00)&(inbuf[15] == 0x00)) ? 1 : 0;
		for (int i = 0; i < 5; i++) {
			wait();
		}
	}
/*
	// CMD 1
	printf("\nSPI SEND CMD1\n");
	flg = 0;

	while(flg == 0) {
		cmd_dispatch(cmd1, inbuf, 8);
		fflush(stdout);
		flg = (inbuf[6] == 0x01) ? 1 : 0;
	}

	// CMD55, ACMD41
	while(1) {
	//for(int i = 0; i < 3; i++) {
		flg = 0;
		while(flg == 0) {
		//for(int j = 0; j < 30; j++) {
			printf("\nSPI SEND CMD55\n");
			cmd_dispatch(cmd55, inbuf, 8);
			fflush(stdout);
			flg = (inbuf[7] == 0x01) ? 1 : 0;
			//if (inbuf[7] == 0x01) break;
			wait();
		}
		printf("\nSPI SEND ACMD41\n");
		fflush(stdout);
		cmd_dispatch(acmd41, inbuf, 8);
		if (inbuf[7] == 0x00) break;
		for (int i = 0; i < 10; i++) {
			wait();
		}
	}	
*/

	// CMD 8
	printf("\nSPI SEND CMD8\n");
	flg = 0;

	cmd_dispatch(cmd8, inbuf, 14);

	while(1);
	return 0;

}

