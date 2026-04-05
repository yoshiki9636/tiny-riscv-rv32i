#define ffconfigHAS_CWD              0
#define ffconfigMAX_FILENAME        256
#define ffconfigLFN_SUPPORT         1
#define ffconfigUNICODE_UTF16_SUPPORT 1
#define ffconfigCACHE_WRITE_THROUGH 1
#define ffconfigFAT_BUFFER_SIZE (8 * 512)

#define ffconfigMALLOC( size ) pvPortMalloc( size )
#define ffconfigFREE( ptr )    vPortFree( ptr )
