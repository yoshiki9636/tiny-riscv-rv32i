// workaround for libc_nano.a
int __errno;

// common modules for all vectors
void uprint( char* buf, int length, int ret ) {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* uart_out = (unsigned int*)0xc000fc00;
    unsigned int* uart_status = (unsigned int*)0xc000fc04;

	for (int i = 0; i < length + ret; i++) {
		unsigned int flg = 1;
		while(flg == 1) {
			flg = *uart_status;
		}
        *uart_out = ((i == length+1)&&(ret == 2)) ? 0x0a :
                    ((i == length)&&(ret == 1)) ? 0x20 :
                    ((i == length)&&(ret == 2)) ? 0x0d : buf[i];
		*led = i;
	}
}

int int_print( char* cbuf, int value, int type ) {
    // type 0 : digit  1:hex
    unsigned char buf[32];
    int ofs = 0;
    int cntr = 0;
    if (type == 0) { // int
        if (value < 0) {
            cbuf[ofs++] = 0x2d;
            value = -value;
        }
        while(value > 0) {
            buf[cntr++] = (unsigned char)(value % 10);
            value = value / 10;
        }
        for(int i = cntr - 1; i >= 0; i--) {
            cbuf[ofs++] = buf[i] + 0x30;
        }
    }
    else { //unsinged int
        unsigned int uvalue = (unsigned int)value;
        while(uvalue > 0) {
            buf[cntr++] = (unsigned char)(uvalue % 10);
            uvalue = uvalue / 10;
        }
        for(int i = cntr - 1; i >= 0; i--) {
            cbuf[ofs++] = buf[i] + 0x30;
        }
    }
    return ofs;
}

int double_print( char* cbuf, double value, int digit ) {
	// type 0 : digit  1:hex
	unsigned char buf[64];

	int cntr = 0;
	
	if (value < 0) {
		buf[cntr++] = 0xfe; // for minus
		value = -value;
	}
	double mug = 1.0;
	while(value >= mug) {
		mug *= 10.0;
	}
	if (mug == 1.0) {
		buf[cntr++] = 0; // first zero
		buf[cntr++] = 0xff; // for preiod
	}
	mug /= 10.0;
	for(int i = 0; i < digit; i++) {	
		unsigned char a =(unsigned char)(value / mug);
		buf[cntr++] = a;
		value = value - (double)a * mug;
		if (mug == 1.0) {
			buf[cntr++] = 0xff; // for preiod
		}
		mug /= 10.0;
	}
	if (mug >= 1.0) {
		while(mug >= 1.0) {
			unsigned char a =(unsigned char)(value / mug);
			buf[cntr++] = a;
			value = value - (double)a * mug;
			mug /= 10.0;
			if (cntr >= 64) {
				break;
			}
		}
	}
	for(int i = 0; i < cntr; i++) {	
		if (buf[i] == 0xff) {
			cbuf[i] = 0x2e;
		}
		else if (buf[i] == 0xfe) {
			cbuf[i] = 0x2d;
		}
		else {
			cbuf[i] = buf[i] + 0x30;
		}
	}	
	return cntr;	
}

void wait() {
    unsigned int timer,timer2;
    timer = 0;
    timer2 = 0;
    while(timer2 < LP2) {
        while(timer < LP) {
            timer++;
        }
        timer2++;
    }
}

void pass() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    while(1) {
        wait();
        val++;
        *led = val & 0x7777;
    }
}

