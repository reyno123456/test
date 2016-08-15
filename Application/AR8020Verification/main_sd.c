#include "flash_init.h"
#include "serial.h"
#include "command.h"
#include "stm32f746xx.h"
#include "memory.h"
#include "ff_gen_drv.h"
#include "diskio.h"
#include "sd_diskio.h"
extern int _flash_data_start;
extern int _flash_data_end;
extern int _flash_data_load;


FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */


//char buff[512];
//__attribute__((section(".stack")))  char buff[512];
//unsigned char buff[512]  __attribute__((section(".stack")));
int main()
{
	serial_init();
	serial_puts("now is main function\n");

	unsigned int prioritygroup = 0;
	prioritygroup = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(0, NVIC_EncodePriority(prioritygroup, 1, 1));
	NVIC_EnableIRQ(0);
	NVIC_SetPriorityGrouping(3);
	SysTick_Config(6000);
	prioritygroup = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(-1, NVIC_EncodePriority(prioritygroup, 0xf, 0));


	int i;
	int *source;
	int *dest;

	unsigned int *addr_sdram = (unsigned int *) SDRAM_BASE;
	unsigned int *addr_sram  = (unsigned int *) DTCM0_BASE;
	unsigned int *sram_test  = (unsigned int *) 0x21002000;
	unsigned int *SDRAM_CLK_SEL = (unsigned int *) 0x40b000c8;
	unsigned int *dtcm = (unsigned int *) CPU1_DTCM_BASE;
	*SDRAM_CLK_SEL = 0x00000000;
	//write_reg32((uint32_t *)0xa003008c, 0x00000001); //bypass_enc_en

	source = &_flash_data_start;
	dest = addr_sdram;
	int Index = 0;
	for (; source <= &_flash_data_end;)
	{
		*dest = *source;
		dest++;
		source++;
	}

//===========================================
//INIT SD_CARD
//===========================================
	sd_init();

//===========================================
//ERASE SD_CARD
//===========================================
	sd_erase(0x00000000, 4);

//===========================================
//WRITE from SDRAM into SD_CARD
//===========================================
#if 0
	/* write SD */
	dest = addr_sdram;
	dma.BlockSize = 512;
	dma.SrcAddr = (uint32_t)dest;
	dma.NumberOfBytes = 512;
	dma.ListBaseAddr = SRAM_BASE;
	dma.DstAddr = 0x00000000;    /*Dstaddr must be in the block units(512bytes) Ex.dma.DstAddr = 0x00000001 means 0x200 in byte units*/
	errorstate = Card_SD_WriteBlocks_DMA(&sdhandle, &dma);
	if (errorstate != SD_OK) {
		serial_puts("write SD failed!\n");
		return 1;
	}
#endif
	/*Dstaddr must be in the block units(512bytes) Ex.dma.DstAddr = 0x00000001 means 0x200 in byte units*/
	//sd_write(0x00000000, addr_sdram, 1);
//===========================================
//READ from SD_CARD into SRAM
//===========================================
#if 1
// #define READNUM 512
// 	for (i = 0; i < (READNUM / 4); ++i)
// 	{
// 		*dtcm = 0;
// 		dtcm++;
// 	}

	char buff[512] ;
	for (int i = 0; i < 512; ++i)
	{
		buff[i] = 0;
	}
	serial_puts("buff addr = ");
	print_str(buff);
	serial_putc('\n');
	//dtcm  = (unsigned int *) CPU1_DTCM_BASE;
	//serial_puts("Before read\n");
	// for (i = 0; i < READNUM; ++i)
	// {
	// 	serial_puts("data = ");
	// 	print_str(*dtcm);
	// 	serial_putc('\n');
	// 	dtcm++;
	// }
	//dtcm  = (unsigned int *) CPU1_DTCM_BASE;
	// /* read SD*/
	// dma.BlockSize = 512;
	// dma.SrcAddr = (uint32_t )0x00000000;  /* the byte units */
	// dma.NumberOfBytes = 100;
	// dma.ListBaseAddr = SRAM_BASE;
	// dma.DstAddr = (uint32_t)dtcm;
	// errorstate = Card_SD_ReadBlocks_DMA(&sdhandle, &dma);
	// if (errorstate != SD_OK) {
	// 	serial_puts("read SD failed!\n");
	// 	return 1;
	// }
	sd_read(buff,  0x00000000, 1);
	for (int i = 0; i < 512; ++i)
	{
		serial_puts("buff = ");
		print_str(buff[i]);
		serial_putc('\n');
	}
	//dtcm  = (unsigned int *) CPU1_DTCM_BASE;
	// serial_puts("After read\n");
	// for (i = 0; i < (READNUM / 4); ++i)
	// {
	// 	serial_puts("data = ");
	// 	print_str(*dtcm);
	// 	serial_putc('\n');
	// 	dtcm++;
	// }
#endif


//===========================================
//DISABLE SD_CARD
//===========================================
	// errorstate = Card_SD_DeInit(&sdhandle);
	// if (errorstate != SD_OK) {
	// 	serial_puts("deinit SD failed!\n");
	// 	return 1;
	// }
	//sd_deinit();


//===========================================
//TEST
//===========================================
#if 0
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread;                     /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	uint8_t rtext[100];                                   /* File read buffer */
	/*##-1- Link the micro SD disk I/O driver ##################################*/
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
	{
		serial_puts("Link success!\n");
		/*##-2- Register the file system object to the FatFs module ##############*/
		if ((res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
		{
			/* FatFs Initialization Error */
			serial_puts("f_mount = ");
			print_str(res);
			serial_putc('\n');
			serial_puts("f_mount error!\n");
		}
		else
		{
			serial_puts("f_mount success!\n");
			/*##-3- Create a FAT file system (format) on the logical drive #########*/
			/* WARNING: Formatting the uSD card will delete all content on the device */
			if (f_mkfs((TCHAR const*)SDPath, 0, 0) != FR_OK)
			{
				/* FatFs Format Error */
				serial_puts("f_mkfs error!\n");
			}
			else
			{
				serial_puts("f_mkfs success!\n");
				/*##-4- Create and Open a new text file object with write access #####*/
				if (f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
				{
					/* 'STM32.TXT' file Open for write Error */
					serial_puts("f_open error!\n");
				}

				else
				{
					serial_puts("f_open success!\n");
					/*##-5- Write data to the text file ################################*/
					res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

					if ((byteswritten == 0) || (res != FR_OK))
					{
						/* 'STM32.TXT' file Write or EOF Error */
						serial_puts("f_write error!\n");
					}
					else
					{
						serial_puts("f_write success!\n");
						/*##-6- Close the open text file #################################*/
						f_close(&MyFile);

						/*##-7- Open the text file object with read access ###############*/
						if (f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
						{
							/* 'STM32.TXT' file Open for read Error */
							serial_puts("f_open error!\n");
						}
						else
						{
							/*##-8- Read data from the text file ###########################*/
							res = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);

							if ((bytesread == 0) || (res != FR_OK))
							{
								/* 'STM32.TXT' file Read or EOF Error */
								serial_puts("f_read error!\n");
							}
							else
							{
								/*##-9- Close the open text file #############################*/
								f_close(&MyFile);

								/*##-10- Compare read data with the expected data ############*/
								if ((bytesread != byteswritten))
								{
									/* Read data is different from the expected data */
									serial_puts("f_close error!\n");
								}
								else
								{
									/* Success of the demo: no error occurrence */
									serial_puts("f_close success!\n");
								}
							}
						}
					}
				}
			}
		}
	}

	/*##-11- Unlink the micro SD disk I/O driver ###############################*/
	res = FATFS_UnLinkDriver(SDPath);
#endif

	serial_puts("Test SD SUCCEED!\n");
	return 0;
}