//===========================================================
// MCS-4 Assember / Disassembler / Simulator
//-----------------------------------------------------------
// File Name   : ads4004.c
// Description : Main Program
//-----------------------------------------------------------
// History :
// Rev.00 2015.08.04 M.Maruyama Start Coding
// Rev.01 2015.09.13 M.Maruyama First Release
// Rev.02 2016.08.06 M.Maruyama Fixed Multiple Literals and Comment
//-----------------------------------------------------------
// Copyright (C) 2015-2016 M.Maruyama
//-----------------------------------------------------------
// Intel has licensed the use of the MCS-4 related materials
// under a Creative Commons license. It is available now 
// 'By-attribution, Non-Commercial, Share-Alike' (BY-NC-SA)
// as described below:
// http://creativecommons.org/licenses/by-nc-sa/3.0/legalcode 
// This project is derived from the documentations below;
//   (1) MCS-4 Micro Computer Set Users Manual,
//       Rev.4, Feb 1973, Intel Corporation.
//   (2) MCS-4 Assembly Language Programming Manual,
//       Preliminay Edition, Dec 1973, Intel Corporation.
// so it carries the same Creative Commons license (BY-NC-SA). 
//===========================================================

//#include <getopt.h>
//#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

//=====================
// Defines
//=====================
//-----------------------------------------------------------------------
// Debug Level
enum DEBUG_LVL {DEBUG_NONE, DEBUG_MIN, DEBUG_MID, DEBUG_MAX};
#define DEBUG_LEVEL DEBUG_NONE
//-----------------------------------------------------------------------
// Command Line Option
enum MCS4_FUNC   {FUNC_ASM, FUNC_DIS, FUNC_SIM};
enum MCS4_OPT    {OPT_OBJ, OPT_LIS, OPT_LOG};
enum MCS4_OPTARG {OPT_NO, OPT_YES};
typedef struct
{
    uint32_t func;
    uint32_t opt_obj;
    uint32_t opt_lis;
    uint32_t opt_log;
    uint8_t *opt_obj_name;
    uint8_t *opt_lis_name;
    uint8_t *opt_log_name;
    uint8_t *input_file_name;
} sOPTION;
//-----------------------------------------------------------------------
// Label Information
#define LABELROOT 256 // hash count
struct label
{
    uint8_t      *name;  // label name
    int32_t       value; // label value
    struct label *next;  // next list
};
typedef struct label sLabel;
enum RESULT
{
    RESULT_NO, RESULT_OK, RESULT_END, RESULT_ALLOCERR, 
    RESULT_UNDEFINED, RESULT_REDEFINED, RESULT_DIVZERO,
    RESULT_NOMNEMONIC, RESULT_NOOPERAND, RESULT_ILGLHEX,
    RESULT_ROMOVERFLOW, RESULT_ILGLTARGET, RESULT_INTERNALERR,
    RESULT_TRUNCATE
};
//-----------------------------------------------------------------------
// Miscellaneous
#define MAXLEN_WORD 256
#define MAXLEN_LINE 1024
#define MAXROM 4096
//-----------------------------------------------------------------------
// Instructions
typedef struct
{
    uint32_t  index;
    uint8_t  *name;
    uint32_t  len;
    uint32_t  operand;
    uint8_t   opcode;
    uint8_t   mask;
} sINSTR;
//
enum PSEUDO_CLASS
{
    PSEUDO_NONE, PSEUDO_END, PSEUDO_ORG, PSEUDO_EQU
};
enum OPERAND_CLASS
{
    OPE_NONE, OPE_RONE, OPE_RPAR, OPE_RBTH,
    OPE_IMM4, OPE_RBIM, OPE_AD12, OPE_C4A8,
    OPE_R1A8
};
//
enum INSTR_INDEX
{
    END, ORG, EQU, 
    INC, FIN, ADD, SUB, LD , XCH,
    CLB, CLC, IAC, CMC, CMA, RAL, RAR, TCC, DAC, TCS, STC, DAA, KBP, 
    FIM, LDM, JUN, JIN, JCN, ISZ, JMS, BBL, NOP,
    DCL, SRC, RDM, RD0, RD1, RD2, RD3, RDR,
    WRM, WR0, WR1, WR2, WR3, WMP, WRR, ADM, SBM, WPM
};
const sINSTR INSTR[] =
{
    //name, len, 
    {END, "END", 0, OPE_NONE, 0x00, 0x00},
    {ORG, "ORG", 0, OPE_NONE, 0x00, 0x00},
    {EQU, "EQU", 0, OPE_NONE, 0x00, 0x00},
    //
    {INC, "INC", 1, OPE_RONE, 0x60, 0xf0},
    {FIN, "FIN", 1, OPE_RBTH, 0x30, 0xf1},
    //
    {ADD, "ADD", 1, OPE_RONE, 0x80, 0xf0},
    {SUB, "SUB", 1, OPE_RONE, 0x90, 0xf0},
    {LD , "LD" , 1, OPE_RONE, 0xa0, 0xf0},
    {XCH, "XCH", 1, OPE_RONE, 0xb0, 0xf0},
    //
    {CLB, "CLB", 1, OPE_NONE, 0xf0, 0xff},
    {CLC, "CLC", 1, OPE_NONE, 0xf1, 0xff},
    {IAC, "IAC", 1, OPE_NONE, 0xf2, 0xff},
    {CMC, "CMC", 1, OPE_NONE, 0xf3, 0xff},
    {CMA, "CMA", 1, OPE_NONE, 0xf4, 0xff},
    {RAL, "RAL", 1, OPE_NONE, 0xf5, 0xff},
    {RAR, "RAR", 1, OPE_NONE, 0xf6, 0xff},
    {TCC, "TCC", 1, OPE_NONE, 0xf7, 0xff},
    {DAC, "DAC", 1, OPE_NONE, 0xf8, 0xff},
    {TCS, "TCS", 1, OPE_NONE, 0xf9, 0xff},
    {STC, "STC", 1, OPE_NONE, 0xfa, 0xff},
    {DAA, "DAA", 1, OPE_NONE, 0xfb, 0xff},
    {KBP, "KBP", 1, OPE_NONE, 0xfc, 0xff},
    //
    {FIM, "FIM", 2, OPE_RBIM, 0x20, 0xf1},
    {LDM, "LDM", 1, OPE_IMM4, 0xd0, 0xf0},
    //
    {JUN, "JUN", 2, OPE_AD12, 0x40, 0xf0},
    {JIN, "JIN", 1, OPE_RPAR, 0x31, 0xf1},
    {JCN, "JCN", 2, OPE_C4A8, 0x10, 0xf0},
    {ISZ, "ISZ", 2, OPE_R1A8, 0x70, 0xf0},
    //
    {JMS, "JMS", 2, OPE_AD12, 0x50, 0xf0},
    {BBL, "BBL", 1, OPE_IMM4, 0xc0, 0xf0},
    //
    {NOP, "NOP", 1, OPE_NONE, 0x00, 0xff},
    //
    {DCL, "DCL", 1, OPE_NONE, 0xfd, 0xff},
    {SRC, "SRC", 1, OPE_RBTH, 0x21, 0xf1},
    {RDM, "RDM", 1, OPE_NONE, 0xe9, 0xff},
    {RD0, "RD0", 1, OPE_NONE, 0xec, 0xff},
    {RD1, "RD1", 1, OPE_NONE, 0xed, 0xff},
    {RD2, "RD2", 1, OPE_NONE, 0xee, 0xff},
    {RD3, "RD3", 1, OPE_NONE, 0xef, 0xff},
    {RDR, "RDR", 1, OPE_NONE, 0xea, 0xff},
    {WRM, "WRM", 1, OPE_NONE, 0xe0, 0xff},
    {WR0, "WR0", 1, OPE_NONE, 0xe4, 0xff},
    {WR1, "WR1", 1, OPE_NONE, 0xe5, 0xff},
    {WR2, "WR2", 1, OPE_NONE, 0xe6, 0xff},
    {WR3, "WR3", 1, OPE_NONE, 0xe7, 0xff},
    {WMP, "WMP", 1, OPE_NONE, 0xe1, 0xff},
    {WRR, "WRR", 1, OPE_NONE, 0xe2, 0xff},
    {ADM, "ADM", 1, OPE_NONE, 0xeb, 0xff},
    {SBM, "SBM", 1, OPE_NONE, 0xe8, 0xff},
    {WPM, "WPM", 1, OPE_NONE, 0xe3, 0xff}
};
//-----------------------------------------------------------------------
// MCS4 System Resource
uint8_t ACC; // Accumulator
uint8_t CY;  // Carry/Borrow Flag
uint8_t R[16]; // Index Register
#define RP(x) ((R[x] << 8) | (R[x+1] << 0)) // Pair Register
//
static volatile uint16_t STACK[4];   // PC and Stack
static volatile uint32_t STACK_CUR;  // Current Stack Pointer
#define STACK_NEXT ((STACK_CUR + 1) % 4) // Next Stack Pointer
#define STACK_PREV ((STACK_CUR + 3) % 4) // Prev Stack Pointer
#define PC (STACK[STACK_CUR]) // Program Counter
//
uint32_t ROM_PORT_IN [16]; // ROM Input Port 4bits
uint32_t ROM_PORT_OUT[16]; // ROM Output Port 4bits
uint32_t RAM_PORT_OUT[32]; // RAM Output Port 4bits
uint32_t TEST = 0; // TEST pin on i4004 pin10
//
uint16_t REG_DCL; // 4bits CM_RAM[4]
int16_t  REG_SRC; // 8bits {RAM_CHIP[2](X2), RAM_REG[2](X3), RAM_CHAR[4](X3)} for RAM
                  //       {ROM_CHIP[4](X2), don't care[4](X3)              } for ROM
#define ROM_CHIP (REG_SRC >> 4)                          // 4bits
#define RAM_BANK ((REG_DCL==4)?3:(REG_DCL==3)?4:REG_DCL) // 3bits
#define RAM_CHIP ((RAM_BANK << 2) | (REG_SRC >> 6))      // 5bits 
#define RAM_ADDR ((RAM_BANK << 8) | (REG_SRC))           // 11bits
#define RSC_ADDR(s) ((RAM_BANK << 6) | ((REG_SRC >> 2) & 0xfc) | (s & 0x03))
uint8_t RAM[2048]; // Address = {BANK[3], CHIP[2], REG[2], CHAR[4]}
uint8_t RSC[ 512]; // Address = {BANK[3], CHIP[2], REG[2], STAT[2]} RAM Status Character
//-----------------------------------------------------------------------
// Access Utility
#define SAMEPAGE(pc, n8) (((pc) & 0x0ff) == 0xff)?               \
                    ((((pc) + 0x100) & 0x0f00) + ((n8) & 0x0ff)) \
                  : ((((pc) + 0x000) & 0x0f00) + ((n8) & 0x0ff))
#define SAMEPAGE2(pc, n8) (((pc) & 0x0ff) == 0xfe)?               \
                    ((((pc) + 0x100) & 0x0f00) + ((n8) & 0x0ff)) \
                  : ((((pc) + 0x000) & 0x0f00) + ((n8) & 0x0ff))
//-----------------------------------------------------------------------

//=====================
// Function Prototype
//=====================
uint32_t Check_Space(uint8_t **ppline);
uint32_t Check_Word(uint8_t **ppline, uint8_t *word);
uint32_t Parse_Expression(uint8_t **ppline, int32_t *value);

//=====================
// Globals
//=====================
sLabel *Label_Root[LABELROOT];
//uint8_t ROM[MAXROM];
// rom data on 0x000c0000
static volatile uint8_t* ROM = (uint8_t*)0x000c0000;
uint32_t pc_max = 10000;
static volatile uint32_t ctrl_c = 0; // originally static volatile

//=============================
// Interrupt Hander for CTRL-C
//=============================
//void Interrupt_Handler(int dummy)
//{
    //ctrl_c = 1;
//}
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
		if (tmp == 0x02) {
			ctrl_c = 1;
		}
		else {
        	tmpchar[tmpnum] = tmp;
        	tmpnum++;
		}
        //uprint((char*)tmpchar, tmpnum, 1);
        *int_clr = 0;
    }
}

//=====================
// DEBUG printf
//=====================
/*
void DEBUG_printf(uint32_t debug_level, char *format, ...)
{
    if (debug_level < DEBUG_LEVEL)
    {
        uint8_t str[1024];
        va_list arg;
        va_start(arg, format);
        vsnprintf(str, 1024, format, arg);
        va_end(arg);
        //fprintf(stdout, "%s", str);
		printf(str);
    }
}
*/

//=============================================================
//-------------------------------------------------------------
// Assembler Routine
//-------------------------------------------------------------
//=============================================================

//====================
// Initialize ROM
//====================
void ROM_Init(void)
{
    uint32_t pc;
    for (pc = 0; pc < MAXROM; pc++) ROM[pc] = 0;
    pc_max = 10000;
}

//===========
// Set ROM
//===========
uint32_t ROM_Set(uint32_t pc, uint8_t data)
{
    uint32_t result = RESULT_OK;
	printf("xa ");
    
    if (pc < MAXROM)
    {
        ROM[pc] = data;
        pc_max = (pc >= pc_max)? pc + 1 : pc_max;
    }
    else
    {
        result = RESULT_ROMOVERFLOW;
    }
    return result;
}

//=============================
// Change String to Upper Case
//=============================
void Change_Upper(uint8_t *str)
{
    uint8_t ch;
    while((ch = *str) != '\0')
    {
        *str = toupper(*str);
        str++;
    }
}

//=============================
// Save a String in Somewhere
//=============================
uint8_t *strsave(uint8_t *s)
{
    uint8_t *p;

    if ((p = (uint8_t*)malloc(strlen(s) + 1)) != NULL) strcpy(p, s);
    return p;
}

//========================
// Initialize Label Table
//========================
void Label_Init(void)
{
    uint32_t i;
    for (i = 0; i < LABELROOT; i++) Label_Root[i] = NULL; 
}

//============================
// Calculate Label Hash Value
//============================
uint32_t Label_Hash(uint8_t *str)
{
    uint32_t hashval;

    for (hashval = 0; *str != '\0';) hashval = hashval + (uint32_t)(*str++);
    return hashval % LABELROOT;
}

//======================
// Lookup Label Table
//======================
sLabel *Label_Lookup(uint8_t *str)
{
    sLabel *plabel;

    for (plabel = Label_Root[Label_Hash(str)]; plabel != NULL; plabel = plabel->next)
    {
        if (strcmp(str, plabel->name) == 0) return plabel; // found
    }
    return NULL; // not found
}

//======================
// Insert Label Table
//======================
uint32_t Label_Insert(uint8_t *name, int32_t value)
{
    uint32_t result = RESULT_NO;
    sLabel *plabel;
    uint32_t hashval;

    if ((plabel = Label_Lookup(name)) == NULL) // new one?
    {
        plabel = (sLabel*) malloc(sizeof(sLabel));
        if (plabel == NULL) return RESULT_ALLOCERR;
        if ((plabel->name = strsave(name)) == NULL) return RESULT_ALLOCERR;
        hashval = Label_Hash(plabel->name);
        plabel->next = Label_Root[hashval];
        Label_Root[hashval] = plabel;
        //
        plabel->value = value; // set
        return RESULT_OK;
    }
    else // already inserted
    {
        plabel->value = value; // replace
        return RESULT_REDEFINED;
    }
}

//======================
// Dispose Label Table
//======================
void Label_Dispose(void)
{
    uint32_t i;
    sLabel *plabel0, *plabel1;
    uint8_t *name;

    for (i = 0; i < LABELROOT; i++)
    {
        plabel0 = Label_Root[i];
        while (plabel0 != NULL)
        {
            plabel1 = plabel0->next;
            name = plabel0->name;
            free(plabel0);
            free(name);
            plabel0 = plabel1;
        }
        Label_Root[i] = NULL;
    }
}

//=========================
// Store Label in String
//=========================
uint32_t Label_Store(uint8_t **ppline, int32_t value, uint32_t pass)
{
    uint32_t result = RESULT_NO;
    uint8_t label[MAXLEN_WORD];
    
    if (Check_Word(ppline, label))
    {
        result = Label_Insert(label, value);
      //printf(">>>>>>>>%s\n", label);
        if ((**ppline == ',') || (**ppline == ':'))
        {
            (*ppline)++;
            Check_Space(ppline);
        }
    }
    if ((pass == 1) && (result == RESULT_REDEFINED)) result = RESULT_OK;
    return result;
}

//========================
// Check Label in String
//========================
uint32_t Label_Check(uint8_t **ppline, int32_t *value)
{
    uint32_t found = 0;
    uint8_t label[MAXLEN_WORD];
    sLabel *plabel;
    
    *value = 0;
    if (Check_Word(ppline, label))
    {
        plabel = Label_Lookup(label);
        if (plabel)
        {
            found = 1;
            *value = plabel->value;
        }
    }
    return found;
}

//=======================
// Check Space and Skip
//=======================
uint32_t Check_Space(uint8_t **ppline)
{
    uint32_t found = 0; 
    while(1)
    {
        if ((**ppline == ' ') || (**ppline == '\t'))
        {
            (*ppline)++;
            found = 1;
        }
        else
        {
            break;
        }
    }
    return found;
}

//=======================
// Check Vacant Line
//=======================
uint32_t Check_Vacant(uint8_t **ppline)
{
    uint32_t found = 0; 
    Check_Space(ppline);
    if ((**ppline == '\n') || (**ppline == '\r') || (**ppline == '\0') || (**ppline == EOF)) found = 1;
    return found;
}

//================
// Check Comment
//================
uint32_t Check_Comment(uint8_t **ppline)
{
    uint32_t found = 0;
    //
    // Check Comment
    found = (**ppline == ';')? 1 :
            ((*(*ppline+0) == '/') && (*(*ppline+1) == '/'))? 1 : 0;
    // Skip until End of the line
    if (found)
    {
        while(1)
        {
            if (**ppline == '\n') break;
            if (**ppline == '\r') break;
            if (**ppline == '\0') break;
            if (**ppline == EOF ) break;
            (*ppline)++;
        }
    }
    // Return    
    return found;
}

//===============================
// Check Decimal Number in String
//===============================
uint32_t Check_Decimal(uint8_t **ppline, int32_t *value)
{
    uint32_t found = 0;
    int32_t sign;
    
    *value = 0;
    if (((**ppline >= '0') && (**ppline <= '9')) || (**ppline == '+') || (**ppline == '-'))
    {
        found = 1;
        sign = 1;
        if (**ppline == '+') (*ppline)++;
        if (**ppline == '-') {sign = -1; (*ppline)++;}
        while((**ppline >= '0') && (**ppline <= '9'))
        {
            *value = *value * 10 + (int32_t)(**ppline - '0');
            (*ppline)++;
        }
        *value = sign * (*value);
        Check_Space(ppline);
    }
    return found;
}

//===============================
// Check Hex Number in String
//===============================
uint32_t Check_Hex(uint8_t **ppline, int32_t *value)
{
    uint32_t found = 0;
    uint32_t msb = 1;
    
    *value = 0;
    if ((**ppline == '0') && (*(*ppline+1) == 'x'))
    {
        *ppline = *ppline + 2;
        while(1)
        {
            if ((**ppline >= '0') && (**ppline <= '7'))
            {
                found = 1;
                *value = *value * 16 + (int32_t)((*(*ppline)++) - '0');
                msb = 0;
            }
            else if ((**ppline >= '8') && (**ppline <= '9'))
            {
                found = 1;
                *value = (msb)? *value * 16 + (int32_t)((*(*ppline)++) - '0' - 16) :
                                *value * 16 + (int32_t)((*(*ppline)++) - '0');
                msb = 0;
            }
            else if ((**ppline >= 'A') && (**ppline <= 'F'))
            {
                found = 1;
                *value = (msb)? *value * 16 + (int32_t)((*(*ppline)++) - 'A' + 10 - 16) :
                                *value * 16 + (int32_t)((*(*ppline)++) - 'A' + 10);
                msb = 0;
            }
            else if ((**ppline >= 'a') && (**ppline <= 'f'))
            {
                found = 1;
                *value = (msb)? *value * 16 + (int32_t)((*(*ppline)++) - 'a' + 10 - 16) :
                                *value * 16 + (int32_t)((*(*ppline)++) - 'a' + 10);
                msb = 0;
            }
            else
            {
                if (found == 0) *ppline = *ppline - 2; // revert pointer
                break;
            }            
        }
        Check_Space(ppline);
    }
    return found;
}

//=========================
// Check Word in String
//=========================
uint32_t Check_Word(uint8_t **ppline, uint8_t *word)
{
    uint32_t found = 0;
    uint32_t len = 0;
    
    while((**ppline != ' ') && (**ppline != '\t') && (**ppline != '\0')
       && (**ppline != ',') && (**ppline != '\n') && (**ppline != '\r')
       && (**ppline != '(') && (**ppline != ')' ) && (**ppline != '+' )
       && (**ppline != '-') && (**ppline != '*' ) && (**ppline != '/' )
       && (**ppline != ';') && (**ppline != ':' ) && (**ppline != '=' )
       && (**ppline != EOF) && (len < MAXLEN_WORD))
    {
        found = 1;
        *word++ = *(*ppline)++;
        len++;
    }
    *word = '\0';
    Check_Space(ppline);
    return found;
}

//==========================================
// Parse Rule for Expression
//------------------------------------------
// [Expression]---[Term]---->
//              |<--<+>---|
//              |<--<->---|
//
// [Term]---[Section]---->
//        |<---<*>----|
//        |<---</>----|
//
// [Section]---[(]--[Expression]--[)]--->
//           |--------[Number]------->|
//==========================================

//==================
// Parse a Number
//==================
uint32_t Parse_Number(uint8_t **ppline, int32_t *value)
{
    uint8_t *pline;
    //
    if (Check_Hex(ppline, value))
    {
        return RESULT_OK;    
    }
    //
    if (Check_Decimal(ppline, value))
    {
        return RESULT_OK;
    }
    //
    if (Label_Check(ppline, value))
    {
        return RESULT_OK;
    }
    //
    *value = 0;
    return RESULT_UNDEFINED;
    
    //
    if (Check_Vacant(ppline))
    {
        *value = 0;
        return RESULT_NO;
    }
    *value = 0;
    return RESULT_OK; //RESULT_UNDEFINED;
}

//============================
// Parse a Section
//============================
uint32_t Parse_Section(uint8_t **ppline, int32_t *value)
{
    uint32_t result = RESULT_NO;
    uint8_t ch;

    ch = **ppline;
    if (ch == '(')
    {
        (*ppline)++;
        Check_Space(ppline);
        result = Parse_Expression(ppline, value);
        result = (*(*ppline)++ == ')')? result : RESULT_NO;
        Check_Space(ppline);
    }
    else
    {
        result = Parse_Number(ppline, value);
    }
    //
    Check_Space(ppline);
    return result;
}

//==================
// Parse a Term
//==================
uint32_t Parse_Term(uint8_t **ppline, int32_t *value)
{
    uint32_t result = RESULT_NO;
    int32_t  value2;

    value2 = 0;
    //
    result = Parse_Section(ppline, value);
    if ((result != RESULT_OK) && (result != RESULT_UNDEFINED)) return result;
    while(1)
    {
        if (Check_Comment(ppline)) break;
        //
        if (**ppline == '*')
        {
            (*ppline)++;
            Check_Space(ppline);
            result = Parse_Section(ppline, &value2);
            if ((result != RESULT_OK) && (result != RESULT_UNDEFINED)) break;
            *value = *value * value2;
        }
        else if (**ppline == '/')
        {
            (*ppline)++;
            Check_Space(ppline);
            result = Parse_Section(ppline, &value2);
            if (value2 == 0) result = RESULT_DIVZERO;
            if ((result != RESULT_OK) && (result != RESULT_UNDEFINED)) break;
            *value = *value / value2;
        }
        else
        {
            break;
        }
    }
    return result;
}

//======================
// Parse an Expression
//======================
uint32_t Parse_Expression(uint8_t **ppline, int32_t *value)
{
    uint32_t result = RESULT_NO;
    int32_t  value2;
    
    value2 = 0;
    //
    result = Parse_Term(ppline, value);
    if ((result != RESULT_OK) && (result != RESULT_UNDEFINED)) return result;    
    while(1)
    {
        if (Check_Comment(ppline)) break;
        //
        if (**ppline == '+')
        {
            (*ppline)++;
            Check_Space(ppline);
            result = Parse_Term(ppline, &value2);
            if ((result != RESULT_OK) && (result != RESULT_UNDEFINED)) break;
            *value = *value + value2;
        }
        else if (**ppline == '-')
        {
            (*ppline)++;
            Check_Space(ppline);
            result = Parse_Term(ppline, &value2);
            if ((result != RESULT_OK) && (result != RESULT_UNDEFINED)) break;
            *value = *value - value2;
        }
        else
        {
            break;
        }
    }
    return result;
}

//=========================================================
// Check and Ignore Undefined Error after Parse_Expression
//=========================================================
uint32_t Parse_Expression_Check_Undefined(uint32_t result, uint32_t pass, int32_t *value)
{
    if (result == RESULT_UNDEFINED)
    {
        if (pass == 0)
        {
            result = RESULT_OK;
            *value = 0;
        }
    }
    return result;
}

//=====================
// Parse Literal
//=====================
uint32_t Parse_Literal(uint8_t **ppline, uint32_t *pc, uint32_t pass)
{
    uint32_t result = RESULT_NO;
    int32_t value;
    
    while(1)
    {
        if (Check_Vacant(ppline)) break;
        result = Parse_Expression(ppline, &value);
        result = Parse_Expression_Check_Undefined(result, pass, &value);
        if (result == RESULT_NO) break;
        if (result == RESULT_UNDEFINED) break;
        ROM_Set(*pc, (uint8_t)value);
        *pc = *pc + 1;
        if (Check_Comment(ppline)) break;
        if (**ppline == ',')
        {
            (*ppline)++;
            Check_Space(ppline);
        }
        else if (**ppline == '\n') break;
        else if (**ppline == EOF) break;
        else if (**ppline == '\0') break;
      //else break;
    }
    return result;
}

//=====================
// Parse Pseudo
//=====================
uint32_t Parse_Psuedo(uint8_t **ppline, uint8_t *pline_label, uint32_t no_label, uint32_t *pc, uint32_t *pc_bgn, uint32_t pass)
{
    uint32_t result = RESULT_NO;
    uint32_t found = 0;
    uint32_t pseudo = PSEUDO_NONE;
    uint8_t word[MAXLEN_WORD];
    uint8_t *pline_begin = *ppline;
    int32_t  value;

    // Which Pseudo Instruction?
    if (**ppline == '=')
    {
        (*ppline)++;
        Check_Space(ppline);
        pseudo = PSEUDO_EQU;
    }
    else
    {
        found = Check_Word(ppline, word);
        Change_Upper(word);
        if (found)
        {
            if (strcmp(word, "END" ) == 0) pseudo = PSEUDO_END;
            else if (strcmp(word, "ORG" ) == 0) pseudo = PSEUDO_ORG;
            else if (strcmp(word, "EQU" ) == 0) pseudo = PSEUDO_EQU;
        }
    }
    //
    // END?
    if (pseudo == PSEUDO_END) return RESULT_END;
    //
    // EQU or "="?
    if (pseudo == PSEUDO_EQU)
    {
        result = Parse_Expression(ppline, &value);
        result = Parse_Expression_Check_Undefined(result, pass, &value);
        if (result != RESULT_OK) return result;
        if (no_label == 0) // label equation
        {
            result = Label_Store(&pline_label, value, 1); // surpress redefined error
        }
        else // no label means origin
        {
            if (value >= MAXROM) return RESULT_ROMOVERFLOW;
            *pc = value;
            *pc_bgn = value;
        }
        return result;
    }
    //
    // ORG?
    if (pseudo == PSEUDO_ORG)
    {
        result = Parse_Expression(ppline, &value);
        result = Parse_Expression_Check_Undefined(result, pass, &value);
        if (result != RESULT_OK) return result;
        if (value >= MAXROM) return RESULT_ROMOVERFLOW;
        *pc = value;
        *pc_bgn = value;
        return result;
    }

    *ppline = pline_begin;
    return RESULT_NO;
}

//====================
// Parse Mnemonic
//====================
uint32_t Parse_Mnemonic(uint8_t **ppline, uint32_t *instr)
{
    uint32_t found = 0;
    uint32_t i;
    uint8_t word[MAXLEN_WORD];
    uint8_t *pline_init = *ppline;
    uint32_t instr_max;
    
    instr_max = sizeof(INSTR) / sizeof(sINSTR);
    *instr = instr_max;
    found = Check_Word(ppline, word);
    if (found)
    {
        Change_Upper(word);
        found = 0;
        for (i = 0; i < instr_max; i++)
        {
          //printf("i=%d word=%s name=%s\n", i, word, ASM[i].name);
            if (strcmp(word, INSTR[i].name) == 0)
            {
                found = 1;
                *instr = i;
                break;
            }
        }
    }
    if (found == 0) *ppline = pline_init;
    Check_Space(ppline);
    return found;
}

//====================
// Parse Operand RONE
//====================
uint32_t Parse_RONE(uint8_t **ppline, uint32_t *rnum)
{
    uint32_t found = 0;
    uint8_t  word[MAXLEN_WORD];
    
    found = Check_Word(ppline, word);
    if (found)
    {
        Change_Upper(word);
             if (strcmp(word, "0" ) == 0) *rnum = 0;
        else if (strcmp(word, "1" ) == 0) *rnum = 1;
        else if (strcmp(word, "2" ) == 0) *rnum = 2;
        else if (strcmp(word, "3" ) == 0) *rnum = 3;
        else if (strcmp(word, "4" ) == 0) *rnum = 4;
        else if (strcmp(word, "5" ) == 0) *rnum = 5;
        else if (strcmp(word, "6" ) == 0) *rnum = 6;
        else if (strcmp(word, "7" ) == 0) *rnum = 7;
        else if (strcmp(word, "8" ) == 0) *rnum = 8;
        else if (strcmp(word, "9" ) == 0) *rnum = 9;
        else if (strcmp(word, "10") == 0) *rnum = 10;
        else if (strcmp(word, "11") == 0) *rnum = 11;
        else if (strcmp(word, "12") == 0) *rnum = 12;
        else if (strcmp(word, "13") == 0) *rnum = 13;
        else if (strcmp(word, "14") == 0) *rnum = 14;
        else if (strcmp(word, "15") == 0) *rnum = 15;
        else found = 0;
    }
    return found;
}

//====================
// Parse Operand RPAR
//====================
uint32_t Parse_RPAR(uint8_t **ppline, uint32_t *rnum)
{
    uint32_t found = 0;
    uint8_t  word[MAXLEN_WORD];
    
    found = Check_Word(ppline, word);
    if (found)
    {
        Change_Upper(word);
             if (strcmp(word, "0<") == 0) *rnum = 0;
        else if (strcmp(word, "1<") == 0) *rnum = 2;
        else if (strcmp(word, "2<") == 0) *rnum = 4;
        else if (strcmp(word, "3<") == 0) *rnum = 6;
        else if (strcmp(word, "4<") == 0) *rnum = 8;
        else if (strcmp(word, "5<") == 0) *rnum = 10;
        else if (strcmp(word, "6<") == 0) *rnum = 12;
        else if (strcmp(word, "7<") == 0) *rnum = 14;
        else if (strcmp(word, "0P") == 0) *rnum = 0;
        else if (strcmp(word, "1P") == 0) *rnum = 2;
        else if (strcmp(word, "2P") == 0) *rnum = 4;
        else if (strcmp(word, "3P") == 0) *rnum = 6;
        else if (strcmp(word, "4P") == 0) *rnum = 8;
        else if (strcmp(word, "5P") == 0) *rnum = 10;
        else if (strcmp(word, "6P") == 0) *rnum = 12;
        else if (strcmp(word, "7P") == 0) *rnum = 14;
        else found = 0;
    }
    return found;
}

//====================
// Parse Operand RBTH
//====================
uint32_t Parse_RBTH(uint8_t **ppline, uint32_t *rnum)
{
    uint32_t found = 0;
    uint8_t  word[MAXLEN_WORD];
    
    found = Check_Word(ppline, word);
    if (found)
    {
        Change_Upper(word);
             if (strcmp(word, "0" ) == 0) *rnum = 0;
        else if (strcmp(word, "2" ) == 0) *rnum = 2;
        else if (strcmp(word, "4" ) == 0) *rnum = 4;
        else if (strcmp(word, "6" ) == 0) *rnum = 6;
        else if (strcmp(word, "8" ) == 0) *rnum = 8;
        else if (strcmp(word, "10") == 0) *rnum = 10;
        else if (strcmp(word, "12") == 0) *rnum = 12;
        else if (strcmp(word, "14") == 0) *rnum = 14;
        else if (strcmp(word, "0<") == 0) *rnum = 0;
        else if (strcmp(word, "1<") == 0) *rnum = 2;
        else if (strcmp(word, "2<") == 0) *rnum = 4;
        else if (strcmp(word, "3<") == 0) *rnum = 6;
        else if (strcmp(word, "4<") == 0) *rnum = 8;
        else if (strcmp(word, "5<") == 0) *rnum = 10;
        else if (strcmp(word, "6<") == 0) *rnum = 12;
        else if (strcmp(word, "7<") == 0) *rnum = 14;
        else if (strcmp(word, "0P") == 0) *rnum = 0;
        else if (strcmp(word, "1P") == 0) *rnum = 2;
        else if (strcmp(word, "2P") == 0) *rnum = 4;
        else if (strcmp(word, "3P") == 0) *rnum = 6;
        else if (strcmp(word, "4P") == 0) *rnum = 8;
        else if (strcmp(word, "5P") == 0) *rnum = 10;
        else if (strcmp(word, "6P") == 0) *rnum = 12;
        else if (strcmp(word, "7P") == 0) *rnum = 14;
        else found = 0;
    }
    return found;
}

//======================
// Parse Condition Code
//======================
uint32_t Parse_Condition_Code(uint8_t **ppline, uint32_t *cc)
{
    uint32_t found = 0;
    uint8_t  word[MAXLEN_WORD];
    uint8_t  *pline_begin = *ppline;
    
    found = Check_Word(ppline, word);
    if (found)
    {
        Change_Upper(word);
        // TZ(0001), TN(1001), C1(0010), C0(1010), AZ(0100), AN(1100)  
             if (strcmp(word, "TZ") == 0) *cc = 1;
        else if (strcmp(word, "TN") == 0) *cc = 9;
        else if (strcmp(word, "C1") == 0) *cc = 2;
        else if (strcmp(word, "C0") == 0) *cc = 10;
        else if (strcmp(word, "AZ") == 0) *cc = 4;
        else if (strcmp(word, "AN") == 0) *cc = 12;
        else found = 0;
    }
    if (found == 0) *ppline = pline_begin;
    return found;
}

//===================
// Put Error
//===================
uint32_t Put_Error(FILE *fp, uint32_t result, uint32_t pass, uint32_t line)
{
    uint32_t error = 0;    
    uint32_t warning = 0;    
    switch (result)
    {
        case RESULT_NO         : {fprintf(stdout, "ERROR: Illegal Item"); error = 1; break;}
        case RESULT_OK         : {error = 0; break;}
        case RESULT_END        : {error = 0; break;}
        case RESULT_ALLOCERR   : {fprintf(stdout, "ERROR: Internal Memory Allocation Error"); error = 1; break;}
        case RESULT_UNDEFINED  : {if (pass) fprintf(stdout, "ERROR: Syntax Error"); error = pass; break;}
        case RESULT_REDEFINED  : {fprintf(stdout, "ERROR: Redefined Label"); error = 1; break;}
        case RESULT_DIVZERO    : {fprintf(stdout, "ERROR: Div by Zero"); error = 1; break;}
        case RESULT_NOMNEMONIC : {fprintf(stdout, "ERROR: Unknown Mnemonic"); error = 1; break;}
        case RESULT_NOOPERAND  : {fprintf(stdout, "ERROR: Illegal Operand"); error = 1; break;}
        case RESULT_ROMOVERFLOW: {fprintf(stdout, "ERROR: ROM Overflow"); error = 1; break;}
        case RESULT_ILGLTARGET : {fprintf(stdout, "ERROR: Illegal Jump Target"); error = 1; break;}
        case RESULT_INTERNALERR: {fprintf(stdout, "ERROR: Internal Error"); error = 1; break;}
        case RESULT_ILGLHEX    : {fprintf(stdout, "ERROR: Illegal Hex Format"); error = 1; break;}
        case RESULT_TRUNCATE   : {fprintf(stdout, "WARNING: Ignored Upper Bits of Immediate Value"); warning = 1; break;}
        default : {fprintf(stdout, "ERROR: Something Unknown Happened"); error = 1; break;}
    }
    if (error | warning) fprintf(stdout, " in line %d.\n", line);
    return error;
}

//=======================
// Put Assemble List
//=======================
void Put_Assemble_List(FILE *fp, uint8_t *pline, uint32_t pc_bgn, uint32_t pc_nxt)
{
    uint32_t pc;
    uint32_t count;

    fprintf(stdout, "%03X ", pc_bgn);
    if (pc_bgn == pc_nxt) {fprintf(stdout, "     "); pc = pc_bgn;}
    else if ((pc_bgn + 1) == pc_nxt) {fprintf(stdout, "%02X   "  , ROM[pc_bgn]); pc = pc_bgn + 1;}
    else if ((pc_bgn + 2) <= pc_nxt) {fprintf(stdout, "%02X %02X", ROM[pc_bgn], ROM[pc_bgn + 1]); pc = pc_bgn + 2;}
    fprintf(stdout, "   %s", pline);
    if ((pc_bgn + 2) < pc_nxt)
    {
        count = 0;
        for (pc = pc_bgn + 2; pc < pc_nxt; pc++)
        {
            if (count == 0)
            {
                fprintf(stdout, "%03X %02X", pc, ROM[pc]);
                if ((pc + 1) == pc_nxt) fprintf(stdout, "\n");
            }
            if (count == 1) fprintf(stdout, " %02X\n", ROM[pc]);
            count = (count + 1) % 2;
        }
    }
}

//====================
// Put Label Table
//====================
void Put_Label_Table(FILE *fp)
{
    uint32_t i;
    uint32_t addr;
    sLabel *plabel;
    
    fprintf(stdout, "-----Label Table-----\n");
    for (addr = 0; addr < pc_max; addr++)
    {
        for (i = 0; i < LABELROOT; i++)
        {
            for (plabel = Label_Root[i]; plabel != NULL; plabel = plabel->next)
            {
                if (addr == plabel->value)
                {
                    fprintf(stdout, "%-8s = 0x%04X\n", plabel->name, plabel->value);
                }
            }
        }
    }
    fprintf(stdout, "-----End of Label Table-----\n");
}

//======================
// Put Hex Format
//======================
void Put_Hex_Format(FILE *fp)
{
    uint32_t pc;
    uint32_t rest;
    uint32_t i;
    uint32_t checksum;
    
    for (pc = 0; pc < pc_max; pc = pc + 16)
    {
        rest = pc_max - pc;
        rest = (rest >= 16)? 16 : rest;
        fprintf(stdout, ":%02X%04X00", rest, pc);
        checksum = rest + (pc >> 8) + (pc & 0x00ff);
        for (i = 0; i < rest; i++)
        {
            fprintf(stdout, "%02X", ROM[pc+i]);
            checksum = checksum + ROM[pc+i];
        }
        checksum = (0 - checksum) & 0x00ff;
        fprintf(stdout, "%02X\n", checksum);
    }
    fprintf(stdout, ":00000001FF\n");
}

//======================
// Get Hex Format
//======================
uint32_t Get_Hex_Format(FILE *fp)
{
    uint32_t result = RESULT_OK;
    uint8_t  hexline[MAXLEN_LINE];
    uint8_t *pline;
    size_t   len;
    uint8_t  str[MAXLEN_WORD];
    uint32_t bytecount;
    uint32_t addr_bgn;
    uint32_t addr;
    uint32_t data;
    uint32_t checksum, checksum2;
    uint32_t line;

    pc_max = 0;
    line = 1;
    while(1)
    {
        // Get a Line
        if (fgets(hexline, MAXLEN_LINE, fp) == NULL) return 0xffffffff;
        pline = hexline;
        // Start Code
        if (*pline++ != ':') {result = RESULT_ILGLHEX; break;}
        // Byte Count
        str[0] = *pline++;
        str[1] = *pline++;
        str[2] = '\0';
        if (sscanf(str, "%02X", (int*)&bytecount) == EOF) {result = RESULT_ILGLHEX; break;}
        checksum = bytecount;
        // Address
        str[0] = *pline++;
        str[1] = *pline++;
        str[2] = *pline++;
        str[3] = *pline++;
        str[4] = '\0';
        if (sscanf(str, "%04X", (int*)&addr_bgn) == EOF) {result = RESULT_ILGLHEX; break;}
        checksum = checksum + (addr_bgn >> 8) + (addr_bgn & 0x00ff);
        // Record Type
        str[0] = *pline++;
        str[1] = *pline++;
        str[2] = '\0';
        if (strcmp(str, "01") == 0) break; // End of File 
        if (strcmp(str, "00") != 0) {result = RESULT_ILGLHEX; break;}
        // Data
        for (addr = addr_bgn; addr < (addr_bgn + bytecount); addr++)
        {
            str[0] = *pline++;
            str[1] = *pline++;
            str[2] = '\0';
            if (sscanf(str, "%02X", (int*)&data) == EOF) {result = RESULT_ILGLHEX; break;}
            ROM[addr] = data;
            checksum = checksum + data;
            pc_max = ((addr + 1) > pc_max)? (addr + 1) : pc_max;
        }        
        if (result != RESULT_OK) break;
        // Checksum
        str[0] = *pline++;
        str[1] = *pline++;
        str[2] = '\0';
        if (sscanf(str, "%02X", (int*)&checksum2) == EOF) {result = RESULT_ILGLHEX; break;}
        checksum = (0 - checksum) & 0x00ff;
        if (checksum != checksum2) {result = RESULT_ILGLHEX; break;}
        //
        line++;
    }
    Put_Error(stdout, result, 1, line);
    return result;
}

//===================
// Put ROM Dump List
//===================
void Put_ROM_Dump_List(FILE *fp)
{
    uint32_t pc;
    fprintf(stdout, "----ROM Contents pc_max=%03x\n", pc_max);
    for (pc = 0; pc < pc_max; pc++)
    {
        if ((pc % 16) == 0) printf("%03x ", pc);
        fprintf(stdout, "%02x ", ROM[pc]);
        if ( ((pc > 0) && (((pc + 1) % 16) == 0)) || ((pc + 1) == pc_max) ) printf("\n");
    }
    fprintf(stdout, "----End of ROM Contents\n");
}

//===================
// Parse Instruction
//===================
uint32_t Parse_Instruction(uint8_t **ppline, uint32_t *pc, uint32_t pass)
{
    uint32_t result = RESULT_OK;
    uint32_t i;
    uint32_t rnum;
    uint8_t  rom;
    int32_t  cc;
    int32_t  value;

    if (Parse_Mnemonic(ppline, &i))
    {
        //
        // CPU Instruction
        switch(INSTR[i].operand)
        {
            //----------------------------------------------------------------
            case OPE_NONE :
            {
                ROM_Set(*pc, (uint8_t)INSTR[i].opcode);
                *pc = *pc + 1;
                break;
            }
            //----------------------------------------------------------------
            case OPE_RONE :
            {
                if (Parse_RONE(ppline, &rnum))
                {
                    rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | rnum);
                    ROM_Set(*pc, rom);
                    *pc = *pc + 1;
                }
                else
                {
                    result = RESULT_NOOPERAND;
                }
                break;
            }
            //----------------------------------------------------------------
            case OPE_RPAR :
            {
                if (Parse_RPAR(ppline, &rnum))
                {
                    rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | rnum);
                    ROM_Set(*pc, rom);
                    *pc = *pc + 1;
                }
                else
                {
                    result = RESULT_NOOPERAND;
                }
                break;
            }
            //----------------------------------------------------------------
            case OPE_RBTH :
            {
                if (Parse_RBTH(ppline, &rnum))
                {
                    rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | rnum);
                    ROM_Set(*pc, rom);
                    *pc = *pc + 1;
                }
                else
                {
                    result = RESULT_NOOPERAND;
                }
                break;
            }
            //----------------------------------------------------------------
            case OPE_IMM4 :
            {
                result = Parse_Expression(ppline, &value);
                result = Parse_Expression_Check_Undefined(result, pass, &value);
                if (result == RESULT_OK)
                {
                    if ((value < -8) || (value > 15))
                    {
                        result = RESULT_TRUNCATE;
                    }
                    value = value & 0x0f;
                    //
                    rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | value);
                    ROM_Set(*pc, rom);
                    *pc = *pc + 1;
                }
                break;
            }
            //----------------------------------------------------------------
            case OPE_RBIM :
            {
                if (Parse_RBTH(ppline, &rnum))
                {
                    result = Parse_Expression(ppline, &value);
                    result = Parse_Expression_Check_Undefined(result, pass, &value);
                    if (result == RESULT_OK)
                    {
                        if ((value < -128) || (value > 255))
                        {
                            result = RESULT_TRUNCATE;
                        }
                        value = value & 0x00ff;
                        //
                        rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | rnum);
                        ROM_Set(*pc, rom);
                        *pc = *pc + 1;
                        ROM_Set(*pc, (uint8_t)value);
                        *pc = *pc + 1;
                    }
                }
                else
                {
                    result = RESULT_NOOPERAND;
                }
                break;
            }
            //----------------------------------------------------------------
            case OPE_AD12 :
            {
                result = Parse_Expression(ppline, &value);
                result = Parse_Expression_Check_Undefined(result, pass, &value);
                if (result == RESULT_OK)
                {
                    if ((value < 0) || (value > 4095))
                    {
                        result = RESULT_ROMOVERFLOW;
                    }
                }
                if (result == RESULT_OK)
                {
                    rom = (uint8_t)(value >> 8);
                    rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | rom);
                    ROM_Set(*pc, rom);
                    *pc = *pc + 1;
                    ROM_Set(*pc, (uint8_t)(value & 0x00ff));
                    *pc = *pc + 1;
                }
                break;
            }
            //----------------------------------------------------------------
            case OPE_C4A8 :
            {
                if (Parse_Condition_Code(ppline, &cc))
                {
                    result = RESULT_OK;
                }
                else
                {
                    result = Parse_Expression(ppline, &cc);
                    result = Parse_Expression_Check_Undefined(result, pass, &cc);
                    if (result == RESULT_OK)
                    {
                      //printf("pc=%02x cc=%d\n", *pc, cc);
                        cc = (cc < 0)? cc + 16 : cc;
                      //printf("pc=%02x cc=%d\n", *pc, cc);
                        result = ((cc >= 0) && (cc <= 15))? result : RESULT_NOOPERAND; 
                    }
                }
                if (result == RESULT_OK)
                {
                    result = Parse_Expression(ppline, &value);
                    result = Parse_Expression_Check_Undefined(result, pass, &value);
                  //printf("pc=%02x value=%03x\n", *pc, value);
                }
                if ((result == RESULT_OK) && (pass == 1))
                {
                  //printf("pc=%02x value=%03x\n", *pc, value);
                    if ((*pc & 0x00ff) == 0x00fe)
                    {
                        result = (((*pc & 0x0f00) + 0x0100) == (value & 0x0f00))? result : RESULT_ILGLTARGET;
                    }
                    else
                    {
                        result = ((*pc & 0x0f00) == (value & 0x0f00))? result : RESULT_ILGLTARGET;
                    }
                }
                if ((result == RESULT_OK) || (result == RESULT_ILGLTARGET))
                {
                    rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | cc);
                    ROM_Set(*pc, rom);
                    *pc = *pc + 1;
                    ROM_Set(*pc, (uint8_t)(value & 0x00ff));
                    *pc = *pc + 1;
                }
                break;
            }
            //----------------------------------------------------------------
            case OPE_R1A8 :
            {
                if (Parse_RONE(ppline, &rnum))
                {
                    if (result == RESULT_OK)
                    {
                        result = Parse_Expression(ppline, &value);
                        result = Parse_Expression_Check_Undefined(result, pass, &value);
                    }
                    if ((result == RESULT_OK) && (pass == 1))
                    {
                      //printf("pc=%02x value=%03x\n", *pc, value);
                        if ((*pc & 0x00ff) == 0x00fe)
                        {
                            result = (((*pc & 0x0f00) + 0x0100) == (value & 0x0f00))? result : RESULT_ILGLTARGET;
                        }
                        else
                        {
                            result = ((*pc & 0x0f00) == (value & 0x0f00))? result : RESULT_ILGLTARGET;
                        }
                    }
                    if ((result == RESULT_OK) || (result == RESULT_ILGLTARGET))
                    {
                        rom = (uint8_t)((INSTR[i].opcode & INSTR[i].mask) | rnum);
                        ROM_Set(*pc, rom);
                        *pc = *pc + 1;
                        ROM_Set(*pc, (uint8_t)(value & 0x00ff));
                        *pc = *pc + 1;
                    }
                }
                else
                {
                    result = RESULT_NOOPERAND;
                }
                break;
            }
            //----------------------------------------------------------------
            default :
            {
                result = RESULT_INTERNALERR;
                break;
            }
            //----------------------------------------------------------------
        }
    }
    else
    {
        result = RESULT_NO;
    }
    return result;
}

//===================
// Assemble a Line
//===================
uint32_t Assemble_Line(uint8_t *pline, uint32_t *pc, uint32_t *pc_bgn, uint32_t pass)
{
    uint32_t result = RESULT_OK;
    uint32_t no_label;
    uint32_t found;
    uint8_t  *pline_label;
    //
    // 1st Character is a Space?
    no_label = Check_Space(&pline);
    //
    // Vacant Line?
    if (Check_Vacant(&pline)) return result; // RESULT_OK
    //
    // Comment?
    if (Check_Comment(&pline)) return result; // RESULT_OK
    //
    // Label, if 1st Character is not a space
    if (no_label == 0)
    {
        pline_label = pline;
        result = Label_Store(&pline, (int32_t)*pc, pass);
        if (result != RESULT_OK) return result;
    }
    //
    // Pseudo Instruction
    result = Parse_Psuedo(&pline, pline_label, no_label, pc, pc_bgn, pass);
    if (result != RESULT_NO) return 0xffffffff;
    //
    // Vacant Line?
    if (Check_Vacant(&pline)) return RESULT_OK;
    //
    // Comment?
    if (Check_Comment(&pline)) return RESULT_OK;
    //
    // Instruction?
    result = Parse_Instruction(&pline, pc, pass);
    //
    // Literal
    if (result == RESULT_NO)
    {
        result = Parse_Literal(&pline, pc, pass);
    }
    //
    return result;
}

//===================
// Do Assemble
//===================
void Do_Asm(sOPTION *psOPTION)
{
    FILE *fp_src;
    FILE *fp_obj;
    FILE *fp_lis;
    uint8_t fname_src[MAXLEN_WORD];
    uint8_t fname_obj[MAXLEN_WORD];
    uint8_t fname_lis[MAXLEN_WORD];
    uint32_t pass;
    uint8_t *pline;
    size_t len;
    uint32_t pc, pc_bgn, pc_nxt;
    uint32_t result;
    uint32_t line;
    uint32_t error;
    //
    // Open Input Source File
    strncpy(fname_src, psOPTION->input_file_name, MAXLEN_WORD);
    fp_src = fopen(fname_src, "r");
    if (fp_src == NULL)
    {
        fprintf(stdout, "Can't open \"%s\".\n", fname_src);
        return;
    }
    //
    // Open Object Hex File
    if (psOPTION->opt_obj == OPT_YES)
        strncpy(fname_obj, psOPTION->opt_obj_name, MAXLEN_WORD);
    else
        snprintf(fname_obj, MAXLEN_WORD, "%s.hex", psOPTION->input_file_name);
    fp_obj = fopen(fname_obj, "w");
    if (fp_obj == NULL)
    {
        fprintf(stdout, "Can't open \"%s\".\n", fname_obj);
        fclose(fp_src);
        return;
    }
    //
    // Open List File
    if (psOPTION->opt_lis == OPT_YES)
        strncpy(fname_lis, psOPTION->opt_lis_name, MAXLEN_WORD);
    else
        snprintf(fname_lis, MAXLEN_WORD, "%s.lis", psOPTION->input_file_name);
    fp_lis = fopen(fname_lis, "w");
    if (fp_lis == NULL)
    {
        fprintf(stdout, "Can't open \"%s\".\n", fname_lis);
        fclose(fp_src);
        fclose(fp_obj);
        return;
    }
    //
    // Initialize
    Label_Init();
    ROM_Init();
    //
    // Two Pass Loop
    for (pass = 0; pass < 2; pass++)
    {
        printf("-------PASS =%2d\n",pass);
        line = 1;
        rewind(fp_src);
        pline = NULL;
        len = 0;
        pc = 0;
        error = 0;
        while(getline((char**)&pline, &len, fp_src) != -1)
        {
            pc_bgn = pc;
            result = Assemble_Line(pline, &pc, &pc_bgn, pass);
            pc_nxt = pc;
            if (pass) // pass1
            {
                Put_Assemble_List(fp_lis, pline, pc_bgn, pc_nxt);
                Put_Error(fp_lis, result, pass, line);
                error = error + Put_Error(stdout, result, pass, line);
            }
            else // pass0
            {
                if (result == RESULT_REDEFINED)
                {
                    Put_Error(fp_lis, result, pass, line);
                    error = error + Put_Error(stdout, result, pass, line);
                }
            }
            if (result == RESULT_END) {break;}
            line++;
        }
        free(pline);
        if (error) break;
    }
    //
    // Print Label and ROM
    if (error == 0) Put_Hex_Format(fp_obj);
    Put_Label_Table(fp_lis);
  //Put_ROM_Dump_List(stdout);
    //
    // Print Error Count
    //fprintf(fp_lis, "Total Error = %d\n", error);
    fprintf(stdout, "Total Error = %d\n", error);
    //
    // Dispose Label Table
    Label_Dispose();
    //
    // Close Files
    fclose(fp_src);
    fclose(fp_obj);
    fclose(fp_lis);
}

//=============================================================
//-------------------------------------------------------------
// Disassembler Routine
//-------------------------------------------------------------
//=============================================================

//==============================
// Generate Dis Assemble String
//==============================
void String_Disasm(uint8_t *str, uint32_t *addr)
{
    uint32_t data, data2;
    uint32_t len;
    uint32_t i;
    uint32_t instr_max;
    
    if (*addr >= pc_max)
    {
        *str = '\0';
        return;
    }    
    instr_max = sizeof(INSTR) / sizeof(sINSTR);    
    sprintf(str, "%03X", *addr);
    //printf("%03X", *addr);
    data = ROM[(*addr)];
    (*addr)++;
    // Search Instruction Table
    for (i = 0; i < instr_max; i++)
    {
        // Pseudo Code, then Skip
        if (INSTR[i].len == 0) continue;
        // Match?
        if ((data & INSTR[i].mask) == INSTR[i].opcode) break;
    }
    // Found
    if (i < instr_max)
    {
        // Print Data
        sprintf(str, "%s %02X", str, data);
        //printf("%s %02X", str, data);
        if (INSTR[i].len == 1)
        {
            sprintf(str, "%s   ", str);
            //printf("%s   ", str);
        }
        else
        {
             data2 = ROM[(*addr)];
             (*addr)++;
             sprintf(str, "%s %02X", str, data2);
             //printf("%s %02X", str, data2);
        }
        // Print Mnemonic
        sprintf(str, "%s    %s", str, INSTR[i].name);
        //printf("%s    %s", str, INSTR[i].name);
        // Print Operand
        switch(INSTR[i].operand)
        {
            case OPE_RONE :
            {
                sprintf(str, "%s %d", str, data & 0x0f);
                //printf("%s %d", str, data & 0x0f);
                break;
            }
            case OPE_RPAR :
            case OPE_RBTH :
            {
                sprintf(str, "%s %dP", str, (data & 0x0f) >> 1);
                //printf("%s %dP", str, (data & 0x0f) >> 1);
                break;
            }
            case OPE_RBIM :
            {
                sprintf(str, "%s %d", str, data & 0x0f);
                //printf("%s %d", str, data & 0x0f);
                sprintf(str, "%s 0x%02x", str, data2);
                //printf("%s 0x%02x", str, data2);
                break;
            }
            case OPE_IMM4 :
            {
                sprintf(str, "%s 0x%01x", str, data & 0x0f);
                //printf("%s 0x%01x", str, data & 0x0f);
                break;
            }
            case OPE_AD12 :
            {
                data = (data & 0x0f) << 8;
                data = data + data2;
                sprintf(str, "%s 0x%03X", str, data);
                //printf("%s 0x%03X", str, data);
                break;
            }
            case OPE_C4A8 :
            {
                switch (data & 0x0f)
                {
                    case  1 : {sprintf(str, "%s TZ", str); break;}
                    case  9 : {sprintf(str, "%s TN", str); break;}
                    case  2 : {sprintf(str, "%s C1", str); break;}
                    case 10 : {sprintf(str, "%s C0", str); break;}
                    case  4 : {sprintf(str, "%s AZ", str); break;}
                    case 12 : {sprintf(str, "%s AN", str); break;}
                    default : {sprintf(str, "%s 0x%01X", str, data & 0x0f); break;}
                    //case  1 : {printf( "%s TZ", str); break;}
                    //case  9 : {printf( "%s TN", str); break;}
                    //case  2 : {printf( "%s C1", str); break;}
                    //case 10 : {printf( "%s C0", str); break;}
                    //case  4 : {printf( "%s AZ", str); break;}
                    //case 12 : {printf( "%s AN", str); break;}
                    //default : {printf( "%s 0x%01X", str, data & 0x0f); break;}
                }
                sprintf(str, "%s 0x%02X", str, data2);
                //printf("%s 0x%02X", str, data2);
                break;
            }
            case OPE_R1A8 :
            {
                sprintf(str, "%s %d", str, data & 0x0f);
                //printf("%s %d", str, data & 0x0f);
                sprintf(str, "%s 0x%02X", str, data2);
                //printf("%s 0x%02X", str, data2);
                break;
            }
            default : break;
        }
    }
    // Not Found
    else
    {
        sprintf(str, "%s %02X       0x%02X ;???", str, data, data);
        //printf("%s %02X       0x%02X ;???", str, data, data);
    }
}

//=========================
// Put Dis Assemble List
//=========================
void Put_Disasm_List(FILE *fp)
{
    uint32_t addr;
    uint8_t  str[MAXLEN_LINE];
    
    addr = 0;
    while(1)
    {
        if (addr >= pc_max) break;
        String_Disasm(str, &addr);
        fprintf(stdout, "%s\n", str);
    }
}

//===================
// Do Disassemble
//===================
void Do_Dis(sOPTION *psOPTION)
{
    FILE *fp_obj;
    FILE *fp_lis;
    uint8_t fname_obj[MAXLEN_WORD];
    uint8_t fname_lis[MAXLEN_WORD];
    uint32_t error;
    //
    // Open Input Hex File
    strncpy(fname_obj, psOPTION->input_file_name, MAXLEN_WORD);
    fp_obj = fopen(fname_obj, "r");
    if (fp_obj == NULL)
    {
        fprintf(stdout, "Can't open \"%s\".\n", fname_obj);
        return;
    }
    //
    // Open List File
    if (psOPTION->opt_lis == OPT_YES)
        strncpy(fname_lis, psOPTION->opt_lis_name, MAXLEN_WORD);
    else
        snprintf(fname_lis, MAXLEN_WORD, "%s.lis", psOPTION->input_file_name);
    fp_lis = fopen(fname_lis, "w");
    if (fp_lis == NULL)
    {
        fprintf(stdout, "Can't open \"%s\".\n", fname_lis);
        fclose(fp_obj);
        return;
    }
    //
    // Initialize
    ROM_Init();
    //
    // Get Hex File
    if (Get_Hex_Format(fp_obj) == RESULT_OK)
    {
        // Put Dis Assemble List
        Put_Disasm_List(fp_lis);
      //Put_ROM_Dump_List(stdout);
    }
    //
    // Close Files
    fclose(fp_obj);
    fclose(fp_lis);
}

//=============================================================
//-------------------------------------------------------------
// Simulator Routine
//-------------------------------------------------------------
//=============================================================

//===================================
// Access Routines for CPU Resource
//===================================
uint8_t Get_R(uint8_t opropa)
{
    uint8_t r4;
    uint32_t n;
    n = opropa & 0x0f;
    r4 = R[n];
    return r4;
}
void Set_R(uint8_t opropa, uint8_t r4)
{
    uint32_t n;
    n = opropa & 0x0f;
    R[n] = r4 & 0x0f;
}
uint8_t Get_RP(uint8_t opropa)
{
    uint8_t r8;
    uint32_t n;
    n = opropa & 0x0e;
    r8 = (R[n] << 4) | (R[n+1] << 0);
    return r8;
}
void Set_RP(uint8_t opropa, uint8_t r8)
{
    uint32_t n;
    n = opropa & 0x0e;
    R[n  ] = (r8 >> 4) & 0x0f;
    R[n+1] = (r8 >> 0) & 0x0f;
}

//======================
// Input Pin Level
//======================
void Input_Pin_Level(uint8_t *pinname, uint32_t *pin)
{
    uint8_t  cmd[MAXLEN_LINE];
    while(1)
    {
        printf(">>>Input %s Level in Hex (Now %x, RET if unchanged)=", pinname, *pin);
		fflush(stdout);
        //if (fgets(cmd, MAXLEN_LINE, stdin))
        if (scanf("%s",cmd ))
        {
            if (strlen(cmd) == 1)
            {
                if ((*cmd >= '0') && (*cmd <= '9'))
                {
                    *pin = (uint32_t)(*cmd - '0');
                    break;
                }
                else if ((*cmd >= 'A') && (*cmd <= 'F'))
                {
                    *pin = (uint32_t)(*cmd + 10 - 'A');
                    break;
                }
                else if ((*cmd >= 'a') && (*cmd <= 'f'))
                {
                    *pin = (uint32_t)(*cmd + 10 - 'a');
                    break;
                }
				else {
					printf("not a hex");
					fflush(stdout);
				}
            }
            //else if ((strlen(cmd) == 1) && (*cmd == '\n'))
            else if (strlen(cmd) == 0)
            {
                break;
            }
        }
    }
}

//=======================
// Print Simulation Log
//=======================
void Sim_fprintf(FILE *fp, char *format, ...)
{
    uint8_t str[1024];
    va_list arg;
    va_start(arg, format);
    vsnprintf(str, 1024, format, arg);
    va_end(arg);
    //fprintf(stdout, "%s", str);
	printf(str);
    //fprintf(fp, "%s", str);
}

//=====================
// Simulation Reset
//=====================
void Sim_Reset(FILE *fp)
{
    uint32_t i;
    
    ACC = 0;
    CY  = 0;
    for (i = 0; i < 16; i++) R[i] = 0;
    //
    for (i = 0; i < 4; i++) STACK[i] = 0;
    STACK_CUR = 0;
    //
    for (i = 0; i < 16; i++) ROM_PORT_IN [i] = 0;
    for (i = 0; i < 16; i++) ROM_PORT_OUT[i] = 0;
    for (i = 0; i < 32; i++) RAM_PORT_OUT[i] = 0;
    TEST = 0;
    //
    REG_DCL = 0;
    REG_SRC = 0;
    for (i = 0; i < 2048; i++) RAM[i] = 0;
    for (i = 0; i <  512; i++) RSC[i] = 0;
    Sim_fprintf(fp, "### Reset ###\n");
}

//===========================
// Put Simulation CPU Status
//===========================
void Put_Sim_CPU_Status(FILE *fp)
{
    uint32_t i;
    uint8_t  str[MAXLEN_LINE];
    uint32_t addr;
    
    addr = PC;
    String_Disasm(str, &addr);
    Sim_fprintf(stdout, "  PC=%03X ACC=%01X CY=%01X", PC, ACC, CY);
    Sim_fprintf(stdout, " R0-15=");
    for (i = 0; i < 16; i++) Sim_fprintf(stdout, "%01X", R[i]);
    Sim_fprintf(stdout, " DCL=%01X SRC=%02X", REG_DCL, REG_SRC);
	
    //Sim_fprintf(stdout, " dbg=%02X\n", ROM[PC]);
    Sim_fprintf(stdout, "  (%s)\n", str); // disasm result
    //printf("  (%s)\n", str); // disasm result
	fflush(stdout);
}

//==================
// Put RAM Status
//==================
void Put_RAM_Status(FILE *fp, uint32_t bank)
{
    uint32_t chip;
    uint32_t reg;
    uint32_t ch;
    uint32_t adrm, adrs;
    
    for (chip = 0; chip < 4; chip++)
    {
        for (reg = 0; reg < 4; reg++)
        {
            adrm = (bank << 8) | (chip << 6) | (reg << 4);
            adrs = (bank << 6) | (chip << 4) | (reg << 2);
            Sim_fprintf(stdout, "Bank=%01x Chip=%01x Reg=%01x Addr=%03x ", bank, chip, reg, adrm);
            for (ch = 0; ch < 16; ch++)
            {
                Sim_fprintf(stdout, " %01x", RAM[adrm + ch]);
            }
            Sim_fprintf(stdout, "  Status= %01x %01x %01x %01x\n",
            RSC[adrs+0], RSC[adrs+1], RSC[adrs+2], RSC[adrs+3]);
        }
    }
}

//==================
// Put Port Status
//==================
void Put_Port_Status(FILE *fp)
{
    uint32_t bank;
    uint32_t chip;
    
    Sim_fprintf(stdout, "CPU TEST Pin = %01x\n", TEST);
    //
    Sim_fprintf(stdout, "ROM Port Input  (chip : 0~15) =");
    for (chip = 0; chip < 16; chip++)
    {
        Sim_fprintf(stdout, " %01x", ROM_PORT_IN[chip]);
    }
    Sim_fprintf(stdout, "\n");
    //
    Sim_fprintf(stdout, "ROM Port Output (chip : 0~15) =");
    for (chip = 0; chip < 16; chip++)
    {
        Sim_fprintf(stdout, " %01x", ROM_PORT_OUT[chip]);
    }
    Sim_fprintf(stdout, "\n");
    //
    for (bank = 0; bank < 8; bank++)
    {
        Sim_fprintf(stdout, "RAM Port Output Bank = %d (chip : 0~3) =", bank);
        for (chip = 0; chip < 4; chip++)
        {
            Sim_fprintf(stdout, " %01x", RAM_PORT_OUT[bank * 4 + chip]);            
        }
        Sim_fprintf(stdout, "\n");
    }
}

//======================
// Execute Instruction
//======================
void Exec_Instruction(void)
{
    uint32_t opropa, opropa2;
    uint32_t len;
    uint32_t i;
    uint32_t instr_max;
    uint16_t pc_bgn;
    uint8_t  tmp;
    
    instr_max = sizeof(INSTR) / sizeof(sINSTR);
    pc_bgn = PC;
    opropa = ROM[PC++];
    // Search Instruction Table
    for (i = 0; i < instr_max; i++)
    {
        // Pseudo Code, then Skip
        if (INSTR[i].len == 0) continue;
        // Match?
        if ((opropa & INSTR[i].mask) == INSTR[i].opcode) break;
    }
    // Found
    if (i < instr_max)
    {
        switch(INSTR[i].index)
        {
            case INC :{Set_R(opropa, (Get_R(opropa)+1)&0x0f); break;}
            case FIN :{Set_RP(opropa, ROM[SAMEPAGE(pc_bgn, Get_RP(0))]); break;}
            case ADD :{tmp=ACC+Get_R(opropa)+CY; CY=(tmp>>4)&0x01; ACC=tmp&0x0f; break;}
            case SUB :{tmp=ACC+(15-Get_R(opropa))+(CY^1); CY=(tmp>>4)&0x01; ACC=tmp&0x0f; break;}
            case LD  :{ACC=Get_R(opropa); break;}
            case XCH :{tmp=ACC; ACC=Get_R(opropa); Set_R(opropa, tmp); break;}
            case CLB :{ACC=0; CY=0; break;}
            case CLC :{CY=0; break;}
            case IAC :{tmp=ACC+1; CY=(tmp>>4)&0x01; ACC=tmp&0x0f; break;}
            case CMC :{CY=CY^1; break;}
            case CMA :{tmp=15-ACC; ACC=tmp&0x0f; break;}
            case RAL :{tmp=ACC<<1; tmp=tmp|CY; CY=(tmp>>4)&0x01; ACC=tmp&0x0f; break;}
            case RAR :{tmp=(CY<<4)|ACC; CY=tmp&0x01; ACC=(tmp>>1)&0x0f; break;}
            case TCC :{ACC=(CY)?1:0; CY=0;break;}
            case DAC :{tmp=ACC+15; CY=(tmp>>4)&0x01; ACC=tmp&0x0f; break;}
            case TCS :{ACC=(CY)?10:9; CY=0;break;}
            case STC :{CY=1; break;}
            case DAA :{tmp=((ACC>9)|(CY))?ACC+6:ACC; CY=(tmp&0x10)?1:CY; ACC=tmp&0x0f; break;}
            case KBP :{ACC=(ACC<=2)?ACC:(ACC==4)?3:(ACC==8)?4:15; break;}
            case FIM :{opropa2 = ROM[PC++]; Set_RP(opropa, opropa2); break;}
            case LDM :{ACC = opropa & 0x0f; break;}
            case JUN :{opropa2 = ROM[PC++]; PC = (((uint16_t)opropa & 0x0f) << 8) + opropa2; break;}
            case JIN :{PC=SAMEPAGE(pc_bgn, Get_RP(opropa));  break;}
            case JCN :
            {
                uint32_t c[4];
                uint32_t jump;
                TEST = TEST & 0x01;
                c[3] = (opropa >> 3) & 0x01; // C1: Invert Jump Condition
                c[2] = (opropa >> 2) & 0x01; // C2: Jump if ACC==0
                c[1] = (opropa >> 1) & 0x01; // C3: Jump if CY==1
                c[0] = (opropa >> 0) & 0x01; // C4: Jump if TEST=0;
                if (c[0]) Input_Pin_Level("TEST Pin", &TEST);
                // ~C1 & C234 + C1 & ~C234 = C1 ^ C234
                jump = c[3] ^ ((c[2] & (ACC==0)) | (c[1] & (CY == 1)) | (c[0] & (TEST == 0)));
                opropa2 = ROM[PC++];
                if (jump) PC = SAMEPAGE2(PC, opropa2);
                break;
            }
            case ISZ :
            {
                Set_R(opropa, (Get_R(opropa) + 1) & 0x0f);
                opropa2 = ROM[PC++];
                if (Get_R(opropa) != 0) PC = SAMEPAGE2(PC, opropa2);
                break;
            }
            case JMS :
            {
                opropa2 = ROM[PC++];
                STACK_CUR = STACK_NEXT;
                PC = (((uint16_t)opropa & 0x0f) << 8) + opropa2; 
                break;
            }
            case BBL :{ACC=opropa&0x0f; STACK_CUR = STACK_PREV; break;}
            case NOP :{break;}
            case DCL :{REG_DCL = ACC & 0x07; break;}
            case SRC :{REG_SRC = Get_RP(opropa); break;}
            case RDM :{ACC = RAM[RAM_ADDR]; break;}
            case RD0 :{ACC = RSC[RSC_ADDR(0)]; break;}
            case RD1 :{ACC = RSC[RSC_ADDR(1)]; break;}
            case RD2 :{ACC = RSC[RSC_ADDR(2)]; break;}
            case RD3 :{ACC = RSC[RSC_ADDR(3)]; break;}
            case RDR :
            {
                uint8_t  rom[MAXLEN_WORD];
                sprintf(rom, "ROM(%d) Port", ROM_CHIP);
                Input_Pin_Level(rom, &(ROM_PORT_IN[ROM_CHIP]));
                ACC = ROM_PORT_IN[ROM_CHIP];
                break;
            }
            case WRM :{RAM[RAM_ADDR] = ACC; break;}
            case WR0 :{RSC[RSC_ADDR(0)] = ACC; break;}
            case WR1 :{RSC[RSC_ADDR(1)] = ACC; break;}
            case WR2 :{RSC[RSC_ADDR(2)] = ACC; break;}
            case WR3 :{RSC[RSC_ADDR(3)] = ACC; break;}
            case WMP :{RAM_PORT_OUT[RAM_CHIP] = ACC; break;}
            case WRR :{ROM_PORT_OUT[ROM_CHIP] = ACC; break;}
            case ADM :{tmp=ACC+RAM[RAM_ADDR]+CY; CY=(tmp>>4)&0x01; ACC=tmp&0x0f; break;}
            case SBM :{tmp=ACC+(15-RAM[RAM_ADDR])+(CY^1); CY=(tmp>>4)&0x01; ACC=tmp&0x0f; break;}
            case WPM :
            {
                static uint8_t count = 0;
                static uint8_t msb = 0;
                uint16_t rom_addr;
                // Writing Program RAM
                if (ROM_PORT_OUT[14] == 1)
                {
                    if (count == 0)
                    {
                        msb = ACC;
                        count = 1;
                    }
                    else
                    {
                        rom_addr = (ROM_PORT_OUT[15] << 8) | REG_SRC;
                        ROM[rom_addr] = (msb << 4) | ACC;
                        count = 0;
                    }
                }
                // Reading Program RAM
                else
                {
                    rom_addr = (ROM_PORT_OUT[15] << 8) | REG_SRC;
                    if (count == 0)
                    {
                        ROM_PORT_IN[14] = (ROM[rom_addr] >> 4) & 0x0f;
                        count = 1;
                    }
                    else
                    {
                        ROM_PORT_IN[15] = (ROM[rom_addr] >> 0) & 0x0f;
                        count = 0;
                    }
                }
                break;
            }
            default : {fprintf(stdout, "ERROR: Internal Bug.\n"); break;}
        }
    }
    // Illegal Instruction
    else
    {
        // so far, ignore
    }
    
    // Wrap round PC, if any
    PC = PC % pc_max;
}

//==========================
// Simulation Main Routine
//==========================
uint8_t *SIMCMD = "### Q/R/G adr/S/M bnk/P >";
void Sim_Main(FILE *fp)
{
    uint32_t quit = 0;
    uint8_t *pcmd;
    uint8_t cmd[MAXLEN_LINE];
    uint8_t pre[MAXLEN_LINE];
    uint16_t addr;
    uint32_t bank;
	int zzz = MAXLEN_LINE;
    //
    printf("test2\n");
	ROM[0] = 0xf8;
    Sim_Reset(fp);
    Put_Sim_CPU_Status(fp);
    //sprintf(pre, "\n");
    while(1)
    {
        //Sim_fprintf(fp, "%s", SIMCMD);
    	printf("%s",SIMCMD);
		fflush(stdout);
        //if (fgets(cmd, MAXLEN_LINE, stdin))
        //if (getline((char**)&cmd, &zzz, stdin))
        if (scanf(" %s",cmd ))
        {
            //if (*cmd == '\n')
            //{
                //strcpy(cmd, pre);
              //if (*cmd != '\n') printf("%s%s", SIMCMD, cmd);
            //}
            //else
            //{
                //strcpy(pre, cmd);
            //}
            //fprintf(stdout, "%s", cmd);
            pcmd = cmd;
            Change_Upper(pcmd);
            Check_Space(&pcmd);
            switch(*pcmd++)
            {
                case '\n' :
                case '\0' :
                {
                    break;
                }
                case 'Q' :
                {
                    quit = 1;
                    break;
                }
                case 'R' :
                {
                    Sim_Reset(fp);
                    Put_Sim_CPU_Status(fp);
                    break;
                }
                case 'G' :
                {
                    Check_Space(&pcmd);
                    if (Check_Vacant(&pcmd)) {printf("ERROR: Specify Address.\n"); break;}
                    addr = 0;
                    if ((sscanf(pcmd, "%X", (int32_t*)&addr) != 1) || (addr < 0) || (addr > (MAXROM - 1)))
                    {
                        printf("ERROR: Specify Correct Address in Hexadecimal.\n");
                    }
                    else
                    {
                        ctrl_c = 0;
                        while(1)
                        {
                            if (ctrl_c) break;
                            if (PC == addr) break;
                            Exec_Instruction();
                            Put_Sim_CPU_Status(fp);
                        }
                    }
                    break;
                }
                case 'S' :
                {
                    Exec_Instruction();
                    Put_Sim_CPU_Status(fp);
                    break;
                }
                case 'M' :
                {
                    Check_Space(&pcmd);
                    if (Check_Vacant(&pcmd)) {printf("ERROR: Specify Bank.\n"); break;}
                    bank = 0;
                    if ((sscanf(pcmd, "%d", (int32_t*)&bank) != 1) || (bank < 0) || (bank > 7))
                    {
                        printf("ERROR: Specify Correct Bank.\n");
                    }
                    else
                    {
                        Put_RAM_Status(fp, bank);
                    }
                    break;
                }
                case 'P' :
                {
                    Put_Port_Status(fp);
                    break;
                }
                default :
                {
                    fprintf(stdout, "ERROR: Illegal Simulation Command.\n");
                    break;
                }
            }
        }
        if (quit) break;
    }
}

//===================
// Do Simulation
//===================
//void Do_Sim(sOPTION *psOPTION)
void Do_Sim()
{
    FILE *fp_obj;
    FILE *fp_log;
    uint8_t fname_obj[MAXLEN_WORD];
    uint8_t fname_log[MAXLEN_WORD];
    uint32_t error;
    //
    // Open Input Hex File
    //strncpy(fname_obj, psOPTION->input_file_name, MAXLEN_WORD);
/*
    fp_obj = fopen(fname_obj, "r");
    if (fp_obj == NULL)
    {
        fprintf(stdout, "Can't open \"%s\".\n", fname_obj);
        return;
    }
    //
    // Open Log File
    if (psOPTION->opt_log == OPT_YES)
        strncpy(fname_log, psOPTION->opt_log_name, MAXLEN_WORD);
    else
        snprintf(fname_log, MAXLEN_WORD, "%s.sim", psOPTION->input_file_name);
    fp_log = fopen(fname_log, "w");
    if (fp_log == NULL)
    {
        fprintf(stdout, "Can't open \"%s\".\n", fname_log);
        fclose(fp_obj);
        return;
    }
*/
    //
    // Initialize
    //ROM_Init();

	//getchar();
	PC = 0;
    // NEED to put HEX file to ROM[]
    Sim_Main(fp_log);

    //
    // Get Hex File
    //if (Get_Hex_Format(fp_obj) == RESULT_OK)
    //{
        //Sim_Main(fp_log);
    //}
    //
    // Close Files
    //fclose(fp_obj);
    //fclose(fp_log);
}

//=============================================================
//-------------------------------------------------------------
// Main Routine
//-------------------------------------------------------------
//=============================================================

//=====================
// Print Usage
//=====================
void Print_Usage(void)
{
    printf("-----------------------------------------------------------------------\n");
    printf("ADS4004 Command Usage                                                     \n");
    printf("-----------------------------------------------------------------------\n");
    printf("$ ads4004 [options] InputFile                                             \n");
    printf("-----------------------------------------------------------------------\n");
    printf("Function Selector                                                      \n");
    printf("    --asm, -a : Assembler (Default)                                    \n");
    printf("    --dis, -d : Disassembler                                           \n");
    printf("    --sim, -s : Simulator                                              \n");
    printf("-----------------------------------------------------------------------\n");
    printf("Assembler : InputFile is a Source List.                                \n");
    printf("    --obj, -o : Object Hex File (Intel Hex) (Default: InputFile.hex)   \n");
    printf("    --lis, -l : Assemble List (Default: InputFile.asm)                 \n");
    printf("-----------------------------------------------------------------------\n");
    printf("Dis Assembler : InputFile is a Object Hex File.                        \n");
    printf("    --lis, -l : Assemble List (Default: InputFile.dis)                 \n");
    printf("-----------------------------------------------------------------------\n");
    printf("Simulator : InputFile is a Object Hex File.                            \n");
    printf("    --log     : Log File (Default: InputFile.sim)                      \n");
    printf("    [Interactive Commands]                                             \n");
    printf("        Q     : Quit                                                   \n");
    printf("        R     : System Reset                                           \n");
    printf("        G adr : Go until Specified Address                             \n");
    printf("        S     : Step                                                   \n");
    printf("        M bnk : Dump Data RAM in Specified Bank                        \n");
    printf("        P     : Dump I/O Port Status of ROM/RAM                        \n");
    printf("-----------------------------------------------------------------------\n");
}

//=====================
// Parse Command Line
//=====================
uint32_t Parse_Command_Line(int argc, char **argv, sOPTION *psOPTION)
{
    uint32_t c;
    //uint32_t long_option_index = 0;
    int long_option_index = 0;
    uint32_t error = 0;
    //
    // Define Long Option
/*
    static struct option long_option[] =
    {
        {"asm", no_argument      , NULL, 'a'},
        {"dis", no_argument      , NULL, 'd'},
        {"sim", no_argument      , NULL, 's'},
        {"obj", required_argument, NULL, 'o'},
        {"lis", required_argument, NULL, 'l'},
        {"log", required_argument, NULL, 'g'},
        {NULL , no_argument      , NULL, 0  }
    };
*/
    //
    // Initialize
    psOPTION->func = FUNC_ASM;
    psOPTION->opt_obj = OPT_NO;
    psOPTION->opt_lis = OPT_NO;
    psOPTION->opt_log = OPT_NO;
    psOPTION->opt_obj_name = NULL;
    psOPTION->opt_lis_name = NULL;
    psOPTION->opt_log_name = NULL;
    //
    // Parse Option Line
/*
    while ((c = (uint32_t)getopt_long(argc, argv, "adso:l:", long_option, &long_option_index)) != -1)
    {
        switch(c)
        {
            case 'a' :
            {
                psOPTION->func = FUNC_ASM;
                break;
            }
            case 'd' :
            {
                psOPTION->func = FUNC_DIS;
                break;
            }
            case 's' :
            {
                psOPTION->func = FUNC_SIM;
                break;
            }
            case 'o' :
            {
                psOPTION->opt_obj = OPT_YES;
                psOPTION->opt_obj_name = optarg;
                break;
            }
            case 'l' :
            {
                psOPTION->opt_lis = OPT_YES;
                psOPTION->opt_lis_name = optarg;
                break;
            }
            case 'g' :
            {
                psOPTION->opt_log = OPT_YES;
                psOPTION->opt_log_name = optarg;
                break;
            }
            default  :
            {
                fprintf(stdout, "Undefined Option \"%c\", ignored.\n", c);
                error = 1;
                break;
            }
        }
        if (error) break;
    }
    //
    // Input File Name
    if (optind < argc)
    {
        psOPTION->input_file_name = argv[optind];
        optind++;
        while (optind < argc) fprintf (stdout, "Ignored Extra Item \"%s\".\n", argv[optind++]);
    }
    else
    {
        fprintf(stdout, "Input File is not Specified.\n");
        error = 1;
    }
*/
    //
    //DEBUG_printf(DEBUG_MAX, "psOPTION->func    = %d\n", psOPTION->func);
    //DEBUG_printf(DEBUG_MAX, "psOPTION->opt_obj = %d, name = %s\n", psOPTION->opt_obj, psOPTION->opt_obj_name);
    //DEBUG_printf(DEBUG_MAX, "psOPTION->opt_lis = %d, name = %s\n", psOPTION->opt_lis, psOPTION->opt_lis_name);
    //DEBUG_printf(DEBUG_MAX, "psOPTION->opt_log = %d, name = %s\n", psOPTION->opt_log, psOPTION->opt_log_name);
    //DEBUG_printf(DEBUG_MAX, "psOPTION->input_file_name = %s\n", psOPTION->input_file_name);
    //
    return error;
}

//=======================
// Main Routine
//=======================
//int main (int argc, char **argv)
void main ()
{
    //sOPTION option;
    //
    // Configure Interrupt (Ctrl-C)
    //signal(SIGINT, Interrupt_Handler);
    //

    //signal(SIGINT, Interrupt_Handler);
    void (*p_func)();
    //register int mask __asm__("x21");
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

    uprint( "start\n", 7, 0);

	printf("test1\n");
    Print_Usage();
    Do_Sim();
	pass();
/*
    // Parse Command Line
    if (Parse_Command_Line(argc, argv, &option))
    {
        Print_Usage();
        exit(EXIT_FAILURE);
    }
    //
    // Do Each Operation
    switch (option.func)
    {
        case FUNC_ASM : {Do_Asm(&option); break;}
        case FUNC_DIS : {Do_Dis(&option); break;}
        case FUNC_SIM : {Do_Sim(&option); break;}
        default : break;
    }
    exit(EXIT_SUCCESS);
*/
}

//===========================================================
// End of Program
//===========================================================

