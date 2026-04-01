extern FF_Disk_t *FF_RAMDiskInit( char *pcName,
                          uint8_t *pucDataBuffer,
                          uint32_t ulSectorCount,
                          size_t xIOManagerCacheSize );

extern int32_t prvReadRAM( uint8_t *pucDestination,
                           uint32_t ulSectorNumber,
                           uint32_t ulSectorCount,
                           FF_Disk_t *pxDisk );

extern int32_t prvWriteRAM( uint8_t *pucSource,
                            uint32_t ulSectorNumber,
                            uint32_t ulSectorCount,
                            FF_Disk_t *pxDisk );
