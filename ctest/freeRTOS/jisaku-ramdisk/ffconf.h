#define ffconfigHAS_CWD              0
#define ffconfigMAX_FILENAME        256
#define ffconfigLFN_SUPPORT         0
#define ffconfigUNICODE_UTF16_SUPPORT 0
#define ffconfigCACHE_WRITE_THROUGH 0

#define ffconfigMALLOC( size ) pvPortMalloc( size )
#define ffconfigFREE( ptr )    vPortFree( ptr )
