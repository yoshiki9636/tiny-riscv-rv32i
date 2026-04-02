#include "ff_headers.h"
#include "ff_stdio.h"
#include "ffconf.h"
#include "ff_ramdisk.h"
#include "riscv-virt.h"

#define ramHIDDEN_SECTOR_COUNT    8
#define ramPRIMARY_PARTITIONS     1
/* The size of each sector on the disk. */
#define ramSECTOR_SIZE                512UL

/* Only a single partition is used. Partition numbers start at 0. */
#define ramPARTITION_NUMBER            0

/*
In this example:
 - pcName is the name to give the disk within FreeRTOS-Plus-FAT's virtual file system.
 - pucDataBuffer is the start of the RAM buffer used as the disk.
 - ulSectorCount is effectively the size of the disk, each sector is 512 bytes.
 - xIOManagerCacheSize is the size of the IO manager's cache, which must be a
 multiple of the sector size, and at least twice as big as the sector size.
*/

static volatile unsigned int* gpio_out_value = (unsigned int*)0xc000fe10;
static volatile unsigned int* spi_mode = (unsigned int*)0xc000f200;
static volatile unsigned int* spi_sckdiv = (unsigned int*)0xc000f204;
static volatile unsigned int* spi_mosi = (unsigned int*)0xc000f208;
static volatile unsigned int* spi_miso = (unsigned int*)0xc000f20c;

char cmd0[9] = { 0xff, 0x40, 0x00, 0x00, 0x00, 0x00, 0x95, 0xff, 0xff };
char cmd8[17] = { 0xff, 0x48, 0x00, 0x00, 0x01, 0xaa, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char cmd9[17] = { 0xff, 0x49, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char cmd10[17] = { 0xff, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char cmd16[9] = { 0xff, 0x50, 0x00, 0x00, 0x02, 0x00, 0x01, 0xff, 0xff }; // set block size to 512
char cmd17[11] = { 0xff, 0x51, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff }; // read block
char cmd24[11] = { 0xff, 0x58, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff }; // write block
char cmd5541[17] = { 0xff, 0x77, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x69, 0x40, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff };
char cmd58[17] = { 0xff, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char dummy[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

char inbuf[64];
int flg;

char cbuf[64];
int length;

// not used for SDCard
static FF_Error_t prvPartitionAndFormatDisk( FF_Disk_t * pxDisk )
{
    FF_PartitionParameters_t xPartition;
    FF_Error_t xError;

    memset( &xPartition, '\0', sizeof( xPartition ) );
    xPartition.ulSectorCount = pxDisk->ulNumberOfSectors;
    xPartition.ulHiddenSectors = ramHIDDEN_SECTOR_COUNT;
    xPartition.xPrimaryCount = ramPRIMARY_PARTITIONS;
    xPartition.eSizeType = eSizeIsQuota;

    //xError = FF_Partition( pxDisk, &xPartition );
    FF_PRINTF( "FF_Partition: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
	//length = sprintf(cbuf, " %ld",xError );
	//uprint( cbuf, length, 2 );

    if( FF_isERR( xError ) == pdFALSE )
    {
        //xError = FF_Format( pxDisk, ramPARTITION_NUMBER, pdTRUE, pdTRUE );
        FF_PRINTF( "FF_RAMDiskInit: FF_Format: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
		//length = sprintf(cbuf, " %ld",xError );
		//uprint( cbuf, length, 2 );

    }

    return xError;
}

// spi sd command dispatch
int cmd_dispatch(char *outbuf, char *inbuf, int len, int ce_on) {

    // reset miso fifo
    *spi_miso = 0x400;
    unsigned int rdata = 0x200;
    char data;

    // print outbuf
    //printf("\nmosi:");
    for (int i = 0; i < len; i++) {
        //printf(" %02x",outbuf[i]);
    }
    //printf("\n");
    // CE = low
    if ( ce_on > 0) {
        //*gpio_out_value =  0x00;
    }

    if(len < 8) {

        // wiret mosi fiio
        for (int i = 0; i < len; i++) {
            *spi_mosi = outbuf[i];
            ////printf(" %02x",outbuf[i]);
        }
        // read miso fiio
        for (int i = 0; i < len; i++) {
            //rdata = 0x200;
            //while ((rdata & 0x200) == 0x200) {
                rdata = *spi_miso;
                ////printf(" %08x",rdata);
            //}
            data = (char)(rdata & 0xff);
            inbuf[i] = data;
            //printf(" %02x",data);
        }
    }
    else {
        // wiret mosi fiio only 4 data
        for (int i = 0; i < 4; i++) {
            *spi_mosi = outbuf[i];
            //printf(" %02x",outbuf[i]);
        }
        // read and wirte both data
        for (int i = 4; i < len; i++) {
            *spi_mosi = outbuf[i];
            //printf(" %02x",outbuf[i]);
            //rdata = 0x200;
            //while ((rdata & 0x200) == 0x200) {
                rdata = *spi_miso;
                //printf(" %08x",rdata);
            //}
            data = (char)(rdata & 0xff);
            inbuf[i-4] = data;
            //printf(" %02x",data);
        }
        // read miso fiio
        for (int i = 0; i < 4; i++) {
            rdata = 0x200;
            //while ((rdata & 0x200) == 0x200) {
                rdata = *spi_miso;
                //printf(" %08x",rdata);
            //}
            data = (char)(rdata & 0xff);
            inbuf[i+len-4] = data;
            //printf(" %02x",data);
         }
    }
    // CE = high
    if ( ce_on != 2) {
        //*gpio_out_value =  0x01;
    }
    // print inbuf
    //printf("miso:");
    for (int i = 0; i < len; i++) {
        //printf(" %02x",inbuf[i]);
    }
    //printf("\n");
    return len;
}

// spi init
int spi_init() {
    static volatile unsigned int* spi_mode = (unsigned int*)0xc000f200;
    static volatile unsigned int* spi_sckdiv = (unsigned int*)0xc000f204;
    static volatile unsigned int* spi_mosi = (unsigned int*)0xc000f208;
    static volatile unsigned int* spi_miso = (unsigned int*)0xc000f20c;

    static volatile unsigned int* gpio_enable = (unsigned int*)0xc000fe18;
    static volatile unsigned int* gpio_out_value = (unsigned int*)0xc000fe10;
    char dummy[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    char inbuf[64];

    // initialization
    // sck : 250kHz
    ////printf("SPI register initialzation");
    //*spi_sckdiv = 200;
    *spi_sckdiv = 20;
    // CPOL=0 CPHA=0
    // SPI enable, big endian
    //*spi_mode = 0x9;
    *spi_mode = 0x00220009;
    // GPIO[0] for CE : workaround
    //*gpio_out_value =  0x01;
    //*gpio_enable = 0x01;

    // wait over 100 cycles
    for (int i = 0; i < 100; i++) {
        //printf("w");
    }
    // dummy clock with CE=high
    for (int i = 0; i < 5; i++) {
        cmd_dispatch(dummy, inbuf, 8, 0);
    }
    // wait over 100 cycles
    for (int i = 0; i < 100; i++) {
        //printf("w");
    }
    //printf("\n");
    // reset miso fifo
    *spi_miso = 0x400;

    return 0;
}

int prvSDCardInit() {
    ////printf("SPI init\n");
    spi_init();

    // CMD 0
    //printf("\nSPI SEND CMD0\n");
    flg = 0;

    while(flg == 0) {
        cmd_dispatch(cmd0, inbuf, 9, 1);
        flg = (inbuf[8] == 0x01) ? 1 : 0;
    }
    cmd_dispatch(dummy, inbuf, 8, 0);

    // CMD 8
    //printf("\nSPI SEND CMD8\n");
    flg = 0;

    cmd_dispatch(cmd8, inbuf, 17, 1);
    //fflush(stdout);

    cmd_dispatch(dummy, inbuf, 8, 0);

    // CMD 55  ACMD41
    //printf("\nSPI SEND CMD55 ACMD41\n");
    flg = 0;

    while(flg == 0) {
        cmd_dispatch(cmd5541, inbuf, 17, 1);
        flg = ((inbuf[8] == 0x01)&(inbuf[16] == 0x00)) ? 1 : 0;
        //flg = ((inbuf[8] == 0x00)&(inbuf[15] == 0x00)) ? 1 : 0;
        cmd_dispatch(dummy, inbuf, 8, 0);

        for (int i = 0; i < 10; i++) {
        }
    }
    // CMD 58
    //printf("\nSPI SEND CMD58\n");
    cmd_dispatch(cmd58, inbuf, 17, 1);

    // CMD 9
    //printf("\nSPI SEND CMD9\n");
    cmd_dispatch(cmd9, inbuf, 17, 1);

    // CMD 58
    //printf("\nSPI SEND CMD10\n");
    cmd_dispatch(cmd10, inbuf, 17, 1);

    // CMD 16 to change block size = 512
    //printf("\nSPI SEND CMD16\n");
    cmd_dispatch(cmd16, inbuf, 9, 1);

	return 0;
}

FF_Disk_t *FF_RAMDiskInit( char *pcName,
                          uint8_t *pucDataBuffer,
                          uint32_t ulSectorCount,
                          size_t xIOManagerCacheSize )
{
FF_Error_t xError;
FF_Disk_t *pxDisk = NULL;
SD_Disk_t *sdDisk = NULL;
FF_CreationParameters_t xParameters;

	//uprint( "init1", 5, 2 );
	// open SD card 
	prvSDCardInit();
	//uprint( "init2", 5, 2 );

    /* Check the validity of the xIOManagerCacheSize parameter. */
    configASSERT( ( xIOManagerCacheSize % ramSECTOR_SIZE ) == 0 );
    configASSERT( ( xIOManagerCacheSize >= ( 2 * ramSECTOR_SIZE ) ) );

    /* Attempt to allocated the [FF_Disk_t](FF_Disk_t) structure. */
    pxDisk = ( FF_Disk_t * ) pvPortMalloc( sizeof( FF_Disk_t ) );
    sdDisk = ( SD_Disk_t * ) pvPortMalloc( sizeof( SD_Disk_t ) );
	//uprint( "init3", 5, 2 );

    if( pxDisk != NULL )
    {
		//uprint( " B", 2, 0 );
        /* It is advisable to clear the entire structure to zero after it has been
           allocated - that way the media driver will be compatible with future
           FreeRTOS-Plus-FAT versions, in which the FF_Disk_t structure may include
           additional members. */
        memset( pxDisk, '\0', sizeof( FF_Disk_t ) );
        memset( sdDisk, '\0', sizeof( SD_Disk_t ) );
		sdDisk->ulSectorSize = ramSECTOR_SIZE;
		sdDisk->ulSectorCount = ulSectorCount;

        /* The pvTag member of the FF_Disk_t structure allows the structure to be
           extended to also include media specific parameters. The only media
           specific data that needs to be stored in the FF_Disk_t structure for a
           RAM disk is the location of the RAM buffer itself - so this is stored
           directly in the FF_Disk_t's pvTag member. */
        pxDisk->pvTag = ( void * ) sdDisk;

        /* The signature is used by the disk read and disk write functions to
           ensure the disk being accessed is a RAM disk. */
        pxDisk->ulSignature = 0;

        /* The number of sectors is recorded for bounds checking in the read and
           write functions. */
        pxDisk->ulNumberOfSectors = ulSectorCount;

        /* Create the IO manager that will be used to control the RAM disk -
           the FF_CreationParameters_t structure completed with the required
           parameters, then passed into the [FF_CreateIOManager()](FF_CreateIOManager) function. */
        memset (&xParameters, '\0', sizeof xParameters);
        xParameters.pucCacheMemory = NULL;
        xParameters.ulMemorySize = xIOManagerCacheSize;
        xParameters.ulSectorSize = ramSECTOR_SIZE;
        xParameters.fnWriteBlocks = prvWriteRAM;
        xParameters.fnReadBlocks = prvReadRAM;
        xParameters.pxDisk = pxDisk;

        /* The driver is re-entrant as it just accesses RAM using memcpy(), so
           xBlockDeviceIsReentrant can be set to pdTRUE. In this case the
           semaphore is only used to protect FAT data structures, and not the read
           and write function. */
        xParameters.pvSemaphore = ( void * ) xSemaphoreCreateRecursiveMutex();
        xParameters.xBlockDeviceIsReentrant = pdTRUE;

        pxDisk->pxIOManager = FF_CreateIOManager( &xParameters, &xError );
		if (pxDisk->pxIOManager == NULL ) {
			//uprint( " E", 2, 0 );
			length = sprintf(cbuf, " %d",xError);
			//uprint( cbuf, length, 2 );
		}

        if( ( pxDisk->pxIOManager != NULL ) && ( FF_isERR( xError ) == pdFALSE ) )
        {
            /* Record that the RAM disk has been initialised. */
            pxDisk->xStatus.bIsInitialised = pdTRUE;

            /* Create a partition on the RAM disk. NOTE! The disk is only
               being partitioned here because it is a new RAM disk. It is
               known that the disk has not been used before, and cannot already
               contain any partitions. Most media drivers will not perform
               this step because the media will already been partitioned and
               formatted. */
			// not format SD card...
            xError = prvPartitionAndFormatDisk( pxDisk );
			//uprint( " G", 2, 0 );

            if( FF_isERR( xError ) == pdFALSE )
            {
				//uprint( " H", 2, 0 );
                /* Record the partition number the FF_Disk_t structure is, then
                   mount the partition. */
                pxDisk->xStatus.bPartitionNumber = ramPARTITION_NUMBER;

                /* Mount the partition. */
                xError = FF_Mount( pxDisk, ramPARTITION_NUMBER );
				//length = sprintf(cbuf, "err: %ld", xError);
				//uprint( cbuf, length, 0 );

            }

            if( FF_isERR( xError ) == pdFALSE )
            {
				//uprint( " I", 2, 0 );
                /* The partition mounted successfully, add it to the virtual
                   file system - where it will appear as a directory off the file
                   system's root directory. */
                FF_FS_Add( pcName, pxDisk );
            }
			//uprint( " J", 2, 0 );
        }
        else
        {
			//uprint( " C", 2, 0 );
            /* The disk structure was allocated, but the disk's IO manager could
               not be allocated, so free the disk again. */
            //FF_RAMDiskDelete( pxDisk );
            pxDisk = NULL;
        }
    }
	//uprint( " D", 2, 0 );

    return pxDisk;
}


int32_t prvReadRAM( uint8_t *pucDestination,
                           uint32_t ulSectorNumber,
                           uint32_t ulSectorCount,
                           FF_Disk_t *pxDisk )
{
	//uint8_t pucSource[512];

	length = sprintf(cbuf, "   Read disk!! %d %d",  ulSectorNumber, ulSectorCount );
	uprint( cbuf, length, 2 );

	for (int s = ulSectorNumber; s < ulSectorNumber + ulSectorCount; s++) {
    	//printf("\nSPI SEND CMD17 : b000\n");
    	cmd17[2] = (char)((s >> 24) & 0xff);
    	cmd17[3] = (char)((s >> 16) & 0xff); 
    	cmd17[4] = (char)((s >>  8) & 0xff);
    	cmd17[5] = (char)((s      ) & 0xff);
    	cmd_dispatch(cmd17, inbuf, 9, 2);

		//unsigned int value = 0x8;
		// disable interrupt
    	inbuf[0] = 0x00;
    	inbuf[1] = 0x00;
    	while((inbuf[0] != 0xfe)&&(inbuf[1] != 0xfe)) {
        	cmd_dispatch(dummy, inbuf, 2, 2);
    	}
		// enable interrupt
    	for (int i = 0; i < 65; i++) {
        	if (i == 64) {
            	cmd_dispatch(dummy, inbuf, 4, 1);
        	}
        	else {
            	cmd_dispatch(dummy, inbuf, 8, 2);
				for (int b = 0; b < 8; b++) {
    				pucDestination[(s-ulSectorNumber)*512+i*8+b] = inbuf[b];
					//pucSource[i*8+b] = inbuf[b];
				}
				//memcpy(&pucSource[i*8], inbuf, 8);
        	}
    	}
		//for (int b = 0; b < 512; b++) {
    		//pucDestination[(s-ulSectorNumber)*512+b] = pucSource[b];
		//}
		/*
    	memcpy( ( void * ) &pucDestination[(s-ulSectorNumber)*512],
            	( void * ) pucSource,
            	( size_t ) ramSECTOR_SIZE );
		*/

    }

    /* Copy the data from the disk. As this is a RAM disk data can be copied
 using memcpy(). */
    return FF_ERR_NONE;
}


int32_t prvWriteRAM( uint8_t *pucSource,
                            uint32_t ulSectorNumber,
                            uint32_t ulSectorCount,
                            FF_Disk_t *pxDisk )
{

	length = sprintf(cbuf, "  Write disk!! %d %d",  ulSectorNumber, ulSectorCount );
	uprint( cbuf, length, 2 );

	for (int s = ulSectorNumber; s < ulSectorNumber + ulSectorCount; s++) {

    // write 0x0000b000 512bytes
    	//printf("\nSPI SEND CMD24 : b000\n");
    	cmd24[2] = (char)((s >> 24) & 0xff);
    	cmd24[3] = (char)((s >> 16) & 0xff); 
    	cmd24[4] = (char)((s >>  8) & 0xff);
    	cmd24[5] = (char)((s      ) & 0xff);
    	cmd_dispatch(cmd24, inbuf, 9, 2);
	
    	// dummy cycle
    	cmd_dispatch(dummy, inbuf, 2, 2);
	
    	// data token
    	char odbuf[2] = { 0xfe, 0x00 };
    	cmd_dispatch(odbuf, inbuf, 1, 2);

    	// data
    	for (int i = 0; i < 512; i++) {
        	odbuf[0] = (char)(pucSource[(s-ulSectorNumber)*512+i]);
        	cmd_dispatch(odbuf, inbuf, 1, 2);
    	}
    	// dummy CRC
    	odbuf[0] = 0x00;
    	cmd_dispatch(odbuf, inbuf, 2, 2);

    	// wait response
		//unsigned int tmp;
		//unsigned int value = 0x0;
		// save interrupt
		//__asm__ volatile("csrr %0, mie" : "=r"(tmp));
		// disable interrupt
		//__asm__ volatile("csrw mie, %0" : "=r"(value));
    	inbuf[0] = 0x1f;
    	inbuf[1] = 0x1f;
    	while(((inbuf[0] & 0x1f) != 0x05)&&((inbuf[1] & 0x1f) != 0x05)) {
        	cmd_dispatch(dummy, inbuf, 2, 2);
    	}
		// enable interrupt
		//__asm__ volatile("csrw mie, %0" : "=r"(tmp));
    	// wait busy
    	inbuf[7] = 0x00;
    	while(inbuf[7] == 0x00) {
        	cmd_dispatch(dummy, inbuf, 8, 2);
    	}
   	}
    return FF_ERR_NONE;
}


