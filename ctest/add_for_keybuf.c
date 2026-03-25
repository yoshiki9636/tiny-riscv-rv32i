
void __attribute__((interrupt)) interrupt_h() {

    static volatile unsigned int* int_clr = (unsigned int*)0xc000fa04;
    static volatile unsigned int* led = (unsigned int*)0xc000fe00;
    //register int mask __asm__("x21");
    // set mstatus
    unsigned int value;
    unsigned int mepc;
    __asm__ volatile("csrr %0, mcause" : "=r"(value));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    //printf(" %x",value);
    //printf(" %x",mepc);
    //fflush(stdout);


    if (value == 2) {
        __asm__ volatile("csrr %0, mtval" : "=r"(value));
        printf(" %x",value);
        __asm__ volatile("csrr %0, mtvec" : "=r"(value));
        printf(" %x",value);
        fflush(stdout);
        mepc += 4;
        __asm__ volatile("csrw mepc, %0" : "=r"(mepc));
        //pass(); // just for stopping
    }
    else if (( *int_clr & 0x1) != 0) {
        volatile unsigned int* rx_char = (unsigned int*)0xc000fc0c;
        char tmp = (char)(*rx_char & 0xff);
        tmpchar[tmpnum] = tmp;
        tmpnum++;
        *int_clr = 0;
    }
}

int key_interrupt_init() {
    void (*p_func)();
    static volatile unsigned int* led = (unsigned int*)0xc000fe00;
    static volatile unsigned int* int_enable = (unsigned int*)0xc000fa00;
    static volatile unsigned int* int_clr = (unsigned int*)0xc000fa04;

    // for external interrupt enable

    p_func = interrupt_h;
    __asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
    // clear mip
    unsigned int value = 0x0;
    __asm__ volatile("csrw mip, %0" : "=r"(value));
    // set mstatus
    value = 0x8;
    __asm__ volatile("csrw mstatus, %0" : "=r"(value));

    // enable MEIE
    value = 0x800;
    __asm__ volatile("csrw mie, %0" : "=r"(value));

    intflg = 1;
    tmpnum = 0;
    *int_clr = 0;
    *int_enable = 1;

	return 0;
}


