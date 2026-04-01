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

int main() {
	static volatile unsigned int* spi_mode = (unsigned int*)0xc000f200;
	static volatile unsigned int* spi_sckdiv = (unsigned int*)0xc000f204;
	static volatile unsigned int* spi_mosi = (unsigned int*)0xc000f208;
	static volatile unsigned int* spi_miso = (unsigned int*)0xc000f20c;

	char cmd00[8] = { 0xff, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff };
	char cmd55[8] = { 0xff, 0x77, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff };
	char acmd41[8] = { 0xff, 0x69, 0x40, 0x00, 0x00, 0x00, 0x01, 0xff };
	char dummy[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	char tmp = calc_sd_crc7(cmd00);
	cmd00[7] = tmp;
	//tmp = calc_sd_crc7(cmd01);
	//cmd01[5] = tmp;
	// initialization
	// sck : 250kHz
	printf("SPI register initialzation");
	*spi_sckdiv = 500;
	// CPOL=0 CPHA=0
	// SPI enable, big endian
	*spi_mode = 0x9;

	// wait over 100 cycles
	//*spi_mosi = 0xff;
	for (int i = 0; i < 1000; i++) {
		printf("w");
	}

	printf("\nSPI SEND CMD0\n");
	//fflush(stdout);
	
	// wait clear mosi fifo
	printf("\nclear mosi\n");
	while (*spi_mosi != 0x200) {
		printf("w");
	}
	printf("reset miso\n");
	//fflush(stdout);
	// reset miso fifo
	*spi_miso = 0x400;

	int flg = 0;
	while(1) {
		// send CMD0
		printf("write CMD0\n");
		//fflush(stdout);
		for (int i = 0; i < 7; i++) {
			*spi_mosi = cmd00[i];
			printf(" %02x",cmd00[i]);
		}
		// read miso fiio
		printf("\nrad CMD0 result\n");
		//fflush(stdout);
		for (int i = 0; i < 8; i++) {
			unsigned int rdata = 0x200;
			while ((rdata & 0x200) == 0x200) {
				rdata = *spi_miso;
				printf(" %08x",rdata);
				//fflush(stdout);
			}
			char data = (char)(rdata & 0xff);
			printf(" %02x",data);
			if((i == 6)&&(data == 0x01)) {
				printf("\nfind success!!\n");
				flg = 1;
				break;
			}
		}
		printf("\n");

		if(flg == 1) { break; }
	}


	// reset miso fifo
	*spi_miso = 0x400;

	// CMD8 CSD Register read
	printf("\nread CMD8\n");
	char cmd08[8] = { 0xff, 0x48, 0x00, 0x00, 0x01, 0xaa, 0x87 };
	//char cmd08[7] = { 0x48, 0x00, 0x00, 0x01, 0xaa, 0x87, 0xff };
	//tmp = calc_sd_crc7(cmd08);
	//cmd08[5] = tmp;

	for (int i = 0; i < 7; i++) {
		*spi_mosi = cmd08[i];
		//printf(" %02x",cmd08[i]);
	}

	// read 1 bytes
	for (int i = 0; i < 1; i++) {
		unsigned int rdata = 0x200;
		while ((rdata & 0x200) == 0x200) {
			rdata = *spi_miso;
			printf(" %08x",rdata);
			fflush(stdout);
		}
		char data = (char)(rdata & 0xff);
		printf(" %02x",data);
	}
	// write 1byte and read 1byte
	for (int i = 0; i < 16; i++) {
		//write
		*spi_mosi = 0xff;
		//read
		unsigned int rdata = 0x200;
		while ((rdata & 0x200) == 0x200) {
			rdata = *spi_miso;
			printf(" %08x",rdata);
			//fflush(stdout);
		}
		char data = (char)(rdata & 0xff);
		printf(" %02x",data);
	}

while(1) {
	flg = 0;
	while(1) {

		// reset miso fifo
		*spi_miso = 0x400;
		// send CMD1
		printf("\nwrite CMD55\n");
		//fflush(stdout);
		for (int i = 0; i < 8; i++) {
			*spi_mosi = cmd55[i];
			printf(" %02x",cmd55[i]);
		}
		// read miso fiio
		printf("\nrad CMD55 result\n");
		//fflush(stdout);
		for (int i = 0; i < 8; i++) {
			unsigned int rdata = 0x200;
			while ((rdata & 0x200) == 0x200) {
				rdata = *spi_miso;
				printf(" %08x",rdata);
				//fflush(stdout);
			}
			char data = (char)(rdata & 0xff);
			printf(" %02x",data);
			if((i == 7)&&(data == 0x01)) {
				printf("\nfind success!!\n");
				flg = 1;
				break;
			}
		}
		printf("\n");
		if(flg == 1) { break; }
	}

	flg = 0;
	while(1) {
		// reset miso fifo
		*spi_miso = 0x400;
		printf("\nwrite ACMD41\n");
		//fflush(stdout);
		for (int i = 0; i < 8; i++) {
			*spi_mosi = acmd41[i];
			printf(" %02x",acmd41[i]);
		}
		// read miso fiio
		printf("\nrad ACND41 result\n");
		//fflush(stdout);
		for (int i = 0; i < 8; i++) {
			unsigned int rdata = 0x200;
			while ((rdata & 0x200) == 0x200) {
				rdata = *spi_miso;
				printf(" %08x",rdata);
				//fflush(stdout);
			}
			char data = (char)(rdata & 0xff);
			printf(" %02x",data);
			if((i == 7)&&(data == 0x00)) {
				printf("\nfind success!!\n");
				flg = 2;
				break;
			}
			else if((i == 7)&&(data == 0x01)) {
				printf("\nfind idle!!\n");
				flg = 1;
				break;
			}
		}

		printf("\n");
		if(flg > 0) { break; }
	}
	//for (int j = 0; j < 8; j++) {
		//for (int i = 0; i < 8; i++) {
			//*spi_mosi = dummy[i];
			//unsigned int rdata = *spi_miso;
			//printf(" %08x",rdata);
		//}
	//}
	if(flg == 2) { break; }
}


	// reset miso fifo
	*spi_miso = 0x400;

	// CMD8 CSD Register read
	printf("\nread CMD8\n");
	// cmd08[7] = { 0x48, 0x00, 0x00, 0x01, 0xaa, 0x87, 0xff };
	//tmp = calc_sd_crc7(cmd08);
	//cmd08[5] = tmp;

	for (int i = 0; i < 7; i++) {
		*spi_mosi = cmd08[i];
		//printf(" %02x",cmd08[i]);
	}

	// read 1 bytes
	for (int i = 0; i < 1; i++) {
		unsigned int rdata = 0x200;
		while ((rdata & 0x200) == 0x200) {
			rdata = *spi_miso;
			printf(" %08x",rdata);
			//fflush(stdout);
		}
		char data = (char)(rdata & 0xff);
		printf(" %02x",data);
	}
	// write 1byte and read 1byte
	for (int i = 0; i < 16; i++) {
		//write
		*spi_mosi = 0xff;
		//read
		unsigned int rdata = 0x200;
		while ((rdata & 0x200) == 0x200) {
			rdata = *spi_miso;
			printf(" %08x",rdata);
			//fflush(stdout);
		}
		char data = (char)(rdata & 0xff);
		printf(" %02x",data);
	}


	// reset miso fifo
	*spi_miso = 0x400;

	// CMD8 CSD Register read
	char cmd09[8] = { 0xff, 0x49, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff };
	printf("\nread CMD9\n");


	for (int i = 0; i < 8; i++) {
		*spi_mosi = cmd09[i];
		//printf(" %02x",cmd09[i]);
	}

	// read 1 bytes
	for (int i = 0; i < 1; i++) {
		unsigned int rdata = 0x200;
		while ((rdata & 0x200) == 0x200) {
			rdata = *spi_miso;
			//printf(" %08x",rdata);
		}
		char data = (char)(rdata & 0xff);
		printf(" %02x",data);
		//fflush(stdout);
	}
	// write 1byte and read 1byte
	for (int i = 0; i < 256; i++) {
		//write
		*spi_mosi = 0xff;
		//read
		unsigned int rdata = 0x200;
		while ((rdata & 0x200) == 0x200) {
			rdata = *spi_miso;
			//printf(" %08x",rdata);
		}
		char data = (char)(rdata & 0xff);
		printf(" %02x",data);
		fflush(stdout);
	}







	while(1);
	return 0;

}

