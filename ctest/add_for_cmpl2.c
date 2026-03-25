// work around for printf

extern char _heap_start;
extern char _heap_end;

char* heap_end = (char*)(&_heap_start);
char* heap_low = (char*)(&_heap_start);
char* heap_top = (char*)(&_heap_end);

char* _sbrk(int incr) {
	char *prev_heap_end;
	char cbuf[64];

	if (heap_end == (char*)0) {
		heap_end = (char*)(&_heap_start);
		heap_low = (char*)(&_heap_start);
		heap_top = (char*)(&_heap_end);
	}
	prev_heap_end = heap_end;

	if (heap_end + incr > heap_top) {
		/* Heap and stack collision */
		uprint( "srbk collision : ", 17, 0 );
		int length = int_print( cbuf, (unsigned int)heap_end, 1 );
		uprint( cbuf, length, 2 );
		uprint( "incr : ", 7, 0 );
		length = int_print( cbuf, (unsigned int)incr, 1 );
		uprint( cbuf, length, 2 );
		return (char *)0;
	}

	heap_end += incr;

	//uprint( "srbk : ", 7, 0 );
	//int length = int_print( cbuf, (unsigned int)heap_end, 1 );
	//uprint( cbuf, length, 2 );
	return (char*) prev_heap_end;
}

int _write(int file, char* ptr, int len)
{
    uprint( ptr, len, 0 );
    return len ;
}

/*
int _read(int file, char* ptr, int len)
{
	volatile unsigned int* rx_char = (unsigned int*)0xc000fc0c;
	volatile unsigned int* led = (unsigned int*)0xc000fe00;
	int char_reg;
	char cbuf[32];

	int i;
	//*rx_char = 0;
	*led = 7;

	for(i = 0; i < len; i++) {
		//uprint( ">", 1, 0 );

		int flg = 0;
		while (flg == 0) {
			char_reg = *rx_char;
			while ((char_reg & 0xf00) == 0) {
				//uprint( "c", 1, 0 );
				char_reg = *rx_char;
				*led = 1;
			}
			*led = 2;
			if ((char_reg & 0x200) != 0) {
				//uprint( "n", 1, 0 );
				//int length = int_print( cbuf, char_reg, 1 );
				//uprint( cbuf, length, 1 );
			}
			else if ((char_reg & 0x100) != 0) {
				//uprint( "b", 1, 0 );
				flg = 1;
				break;
			}
			else {
				//uprint( "e", 1, 0 );
			}
		}
		ptr[i] = (char)(char_reg & 0xff);
		//*rx_char = 0;
		//uprint( ptr, i, 2 );
		if (ptr[i] == '\r') break;
		if (ptr[i] == '\n') break;
	}
	return i;
}
*/

/*
int _read(int file, char* ptr, int len)
{
	volatile unsigned int* rx_char = (unsigned int*)0xc000fc0c;
	volatile unsigned int* led = (unsigned int*)0xc000fe00;
	int char_reg;
	char cbuf[32];

	int i;
	//*rx_char = 0;
	*led = 7;

	for(i = 0; i < len; i++) {
		//uprint( ">", 1, 0 );

		int flg = 0;
		while (flg == 0) {
			char_reg = *rx_char;
			while ((char_reg & 0xf00) == 0) {
				//uprint( "c", 1, 0 );
				char_reg = *rx_char;
				*led = 1;
			}
			*led = 2;
			if ((char_reg & 0x200) != 0) {
				//uprint( "n", 1, 0 );
				//int length = int_print( cbuf, char_reg, 1 );
				//uprint( cbuf, length, 1 );
			}
			else if ((char_reg & 0x100) != 0) {
				//uprint( "b", 1, 0 );
				flg = 1;
				break;
			}
			else {
				//uprint( "e", 1, 0 );
			}
		}
		ptr[i] = (char)(char_reg & 0xff);
		//*rx_char = 0;
		//uprint( ptr, i, 2 );
		char_reg = *rx_char;
		if (ptr[i] == '\r') break;
		//if (ptr[i] == '\n') break;
		//for (int j = 0; j < 5; j++) {
			//char_reg = *rx_char;
			//int length = int_print( cbuf, char_reg, 1 );
			//uprint( cbuf, length, 1 );
		//}

	}
	return i+1;
}
*/

//int getline(char* buffer, int len)
int getline(char** buffer, int* len, FILE *stream)
{
	volatile unsigned int* rx_char = (unsigned int*)0xc000fc0c;
	volatile unsigned int* led = (unsigned int*)0xc000fe00;
	int char_reg;
	char cbuf[32];

	int i;
	//*rx_char = 0;
	*led = 7;

	for(i = 0; i < *len; i++) {
		//uprint( ">", 1, 0 );

		int flg = 0;
		while (flg == 0) {
			char_reg = *rx_char;
			while ((char_reg & 0xf00) == 0) {
				//uprint( "c", 1, 0 );
				char_reg = *rx_char;
				*led = 4;
			}
			*led = 2;
			if ((char_reg & 0x200) != 0) {
				//uprint( "n", 1, 0 );
				//int length = int_print( cbuf, char_reg, 1 );
				//uprint( cbuf, length, 1 );
			}
			else if ((char_reg & 0x100) != 0) {
				//uprint( "b", 1, 0 );
				flg = 1;
				break;
			}
			else {
				//uprint( "e", 1, 0 );
			}
		}
		*buffer[i] = (char)(char_reg & 0xff);
		//*rx_char = 0;
		//uprint( buffer, i, 2 );
		char_reg = *rx_char;
		if (*buffer[i] == '\r') break;
		if (*buffer[i] == '\n') break;
		if (*buffer[i] == '\0') break;

	}
	return i+1;

}

static volatile int intflg = 0;
static volatile int tmpnum = 0;
static volatile char tmpchar[32];

int _read(int file, char* ptr, int len)
{
	volatile unsigned int* rx_char = (unsigned int*)0xc000fc0c;
	volatile unsigned int* led = (unsigned int*)0xc000fe00;
	int char_reg;
	char cbuf[32];

	//int i = 0;
	*led = 7;
	if (intflg == 1) {
		while(tmpnum == 0);
		if (tmpnum > 0) {
			for (int i = 0; i < tmpnum; i++ ) {
				ptr[i] =  tmpchar[i];
			}
			int ret = (int)tmpnum;
			tmpnum = 0;
			return ret;
		}
		else {
			return 0;
		}
	}
	
	// key buffer clear
	//char_reg = *rx_char;

	int flg = 0;
	while (flg == 0) {
		char_reg = *rx_char;
		while ((char_reg & 0xf00) == 0) {
			//uprint( "c", 1, 0 );
			char_reg = *rx_char;
			*led = 1;
		}
		*led = 2;
		if ((char_reg & 0x100) != 0) {
			//uprint( "b", 1, 0 );
			flg = 1;
			break;
		}
	}
	ptr[0] = (char)(char_reg & 0xff);
	//uprint( ptr, i, 2 );
	char_reg = *rx_char;
	//if (ptr[i] == '\r') break;

	return 1;
}


// workaround for using libc_nano.a
void _exit(int a) { while(1) {} }
int _open(void) { return 0; }
int _close(void) { return 0; }
int _lseek(void) { return 0; }
//int _read(void) { return 0; }
//void _write(void) {}
//void _sbrk_r(void) {}
void abort(void) { while(1) {} }
void _kill_r(void) { return;}
int _getpid_r(void) { return -1; }
int _fstat_r(void) { return -1; }
int _isatty_r(void) { return -1; }
int _isatty(void) { return -1; }

