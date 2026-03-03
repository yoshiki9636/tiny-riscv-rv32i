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

// workaround for using libc_nano.a
int _close(void) { return 0; }
int _lseek(void) { return 0; }
int _read(void) { return 0; }
//void _write(void) {}
//void _sbrk_r(void) {}
void abort(void) { while(1) {} }
void _kill_r(void) { return;}
int _getpid_r(void) { return -1; }
int _fstat_r(void) { return -1; }
int _isatty_r(void) { return -1; }
int _isatty(void) { return -1; }

