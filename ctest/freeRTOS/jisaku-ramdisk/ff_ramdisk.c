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

char cbuf[64];
int length;

static FF_Error_t prvPartitionAndFormatDisk( FF_Disk_t * pxDisk )
{
    FF_PartitionParameters_t xPartition;
    FF_Error_t xError;

    /* Create a single partition that fills all available space on the disk. */
    memset( &xPartition, '\0', sizeof( xPartition ) );
    xPartition.ulSectorCount = pxDisk->ulNumberOfSectors;
    xPartition.ulHiddenSectors = ramHIDDEN_SECTOR_COUNT;
    xPartition.xPrimaryCount = ramPRIMARY_PARTITIONS;
    xPartition.eSizeType = eSizeIsQuota;

    /* Partition the disk */
    xError = FF_Partition( pxDisk, &xPartition );
    FF_PRINTF( "FF_Partition: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
	length = sprintf(cbuf, " %d",xError );
	uprint( cbuf, length, 2 );

    if( FF_isERR( xError ) == pdFALSE )
    {
        /* Format the partition. */
        xError = FF_Format( pxDisk, ramPARTITION_NUMBER, pdTRUE, pdTRUE );
        FF_PRINTF( "FF_RAMDiskInit: FF_Format: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
		length = sprintf(cbuf, " %d",xError );
		uprint( cbuf, length, 2 );

    }

    return xError;
}


FF_Disk_t *FF_RAMDiskInit( char *pcName,
                          uint8_t *pucDataBuffer,
                          uint32_t ulSectorCount,
                          size_t xIOManagerCacheSize )
{
FF_Error_t xError;
FF_Disk_t *pxDisk = NULL;
FF_CreationParameters_t xParameters;

    /* Check the validity of the xIOManagerCacheSize parameter. */
    configASSERT( ( xIOManagerCacheSize % ramSECTOR_SIZE ) == 0 );
    configASSERT( ( xIOManagerCacheSize >= ( 2 * ramSECTOR_SIZE ) ) );

    /* Attempt to allocated the [FF_Disk_t](FF_Disk_t) structure. */
    pxDisk = ( FF_Disk_t * ) pvPortMalloc( sizeof( FF_Disk_t ) );

    if( pxDisk != NULL )
    {
		uprint( " B", 2, 0 );
        /* It is advisable to clear the entire structure to zero after it has been
           allocated - that way the media driver will be compatible with future
           FreeRTOS-Plus-FAT versions, in which the FF_Disk_t structure may include
           additional members. */
        memset( pxDisk, '\0', sizeof( FF_Disk_t ) );

        /* The pvTag member of the FF_Disk_t structure allows the structure to be
           extended to also include media specific parameters. The only media
           specific data that needs to be stored in the FF_Disk_t structure for a
           RAM disk is the location of the RAM buffer itself - so this is stored
           directly in the FF_Disk_t's pvTag member. */
        pxDisk->pvTag = ( void * ) pucDataBuffer;

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
			uprint( " E", 2, 0 );
			length = sprintf(cbuf, " %d",xError);
			uprint( cbuf, length, 2 );
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
            xError = prvPartitionAndFormatDisk( pxDisk );
			uprint( " G", 2, 0 );

            if( FF_isERR( xError ) == pdFALSE )
            {
				uprint( " H", 2, 0 );
                /* Record the partition number the FF_Disk_t structure is, then
                   mount the partition. */
                pxDisk->xStatus.bPartitionNumber = ramPARTITION_NUMBER;

                /* Mount the partition. */
                xError = FF_Mount( pxDisk, ramPARTITION_NUMBER );
				length = sprintf(cbuf, "err: %ld", xError);
				uprint( cbuf, length, 0 );

            }

            if( FF_isERR( xError ) == pdFALSE )
            {
				uprint( " I", 2, 0 );
                /* The partition mounted successfully, add it to the virtual
                   file system - where it will appear as a directory off the file
                   system's root directory. */
                FF_FS_Add( pcName, pxDisk );
            }
			uprint( " J", 2, 0 );
        }
        else
        {
			uprint( " C", 2, 0 );
            /* The disk structure was allocated, but the disk's IO manager could
               not be allocated, so free the disk again. */
            //FF_RAMDiskDelete( pxDisk );
            pxDisk = NULL;
        }
    }
	uprint( " D", 2, 0 );

    return pxDisk;
}

int32_t prvReadRAM( uint8_t *pucDestination,
                           uint32_t ulSectorNumber,
                           uint32_t ulSectorCount,
                           FF_Disk_t *pxDisk )
{
uint8_t *pucSource;

	length = sprintf(cbuf, "Read disk!! %d %d",  ulSectorNumber, ulSectorCount );
	uprint( cbuf, length, 2 );

    /* The FF_Disk_t structure describes the media being accessed. Attributes that
 are common to all media types are stored in the structure directly. The pvTag
 member of the structure is used to add attributes that are specific to the media
 actually being accessed. In the case of the RAM disk the pvTag member is just
 used to point to the RAM buffer being used as the disk. */
    pucSource = ( uint8_t * ) pxDisk->pvTag;

    /* Move to the start of the sector being read. */
    pucSource += ( ramSECTOR_SIZE * ulSectorNumber );

    /* Copy the data from the disk. As this is a RAM disk data can be copied
 using memcpy(). */
    memcpy( ( void * ) pucDestination,
            ( void * ) pucSource,
            ( size_t ) ( ulSectorCount * ramSECTOR_SIZE ) );

    return FF_ERR_NONE;
}


int32_t prvWriteRAM( uint8_t *pucSource,
                            uint32_t ulSectorNumber,
                            uint32_t ulSectorCount,
                            FF_Disk_t *pxDisk )
{
uint8_t *pucDestination;

	length = sprintf(cbuf, "Write disk!! %d %d",  ulSectorNumber, ulSectorCount );
	uprint( cbuf, length, 2 );

    /* The FF_Disk_t structure describes the media being accessed. Attributes that
       are common to all media types are stored in the structure directly. The pvTag
       member of the structure is used to add attributes that are specific to the media
       actually being accessed. In the case of the RAM disk the pvTag member is just
       used to point to the RAM buffer being used as the disk. */
    pucDestination = ( uint8_t * ) pxDisk->pvTag;

    /* Move to the start of the sector being written. */
    pucDestination += ( ramSECTOR_SIZE * ulSectorNumber );

    /* Copy the data to the disk. As this is a RAM disk data can be copied
       using memcpy(). */
    memcpy( ( void * ) pucDestination,
            ( void * ) pucSource,
            ( size_t ) ( ulSectorCount * ramSECTOR_SIZE ) );

    return FF_ERR_NONE;
}


