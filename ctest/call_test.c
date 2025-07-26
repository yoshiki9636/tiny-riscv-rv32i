#include <stdio.h>

//#define LP 10
#define LP 10000
#define LP2 200
void pass();
void fail(unsigned int val1, unsigned int val2, unsigned int val3, unsigned int val4, unsigned int val5);
int wait();

int main() {
	unsigned int i = 1;
	unsigned int j = 2;
	unsigned int k = 3;
	unsigned int l = 4;
	unsigned int m = 5;
	unsigned int n = 6;
	unsigned int o = 7;
	unsigned int p = 0x11;
	fail(l,m,n,o,p);

}

void pass() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    while(1) {
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}
		val++;
		*led = val & 0x7777;
    }
}

void fail(unsigned int val1, unsigned int val2, unsigned int val3, unsigned int val4, unsigned int val5) {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    unsigned int sw = 0;
    while(1) {
		*led = 0x0;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = val1 & 0x7777;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = 0x0;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = val2 & 0x7777;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = 0x0;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = val3 & 0x7777;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = 0x0;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = val4 & 0x7777;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = 0x0;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = val5 & 0x7777;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = 0x0;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		*led = 0x7777;
        timer = 0;
		timer2 = 0;
        while(timer2 < LP2) {
            while(timer < LP) {
                timer++;
	    	}
            timer2++;
		}

		val++;
    }
}

int wait() {
    unsigned int timer,timer2;
    timer = 0;
	timer2 = 0;
    while(timer2 < LP2) {
        while(timer < LP) {
            timer++;
    	}
        timer2++;
	}
	return timer2;
}

