#include "sd_host.h"
#include "test_sd.h"
//#include "string.h"
#include "stm32f746xx.h"
#include "debuglog.h"
#include "flash_init.h"
#include "ff.h"
extern int _flash_data_start;
extern int _flash_data_end;
extern int _flash_data_load;

#define READNUM 5120

unsigned int *addr_sdram = (unsigned int *)0x81000000;
// __attribute__((section(".stack")))  char buff[512];
 //unsigned char buff[512]  __attribute__((section(".stack")));
// FATFS SDFatFs;  /* File system object for SD card logical drive */
// FIL MyFile;     /* File object */
// char SDPath[4]; /* SD card logical drive path */

void TestWR(unsigned int length)
{
	//unsigned char buff[512];
	int i, *source, *dest;
	source = &_flash_data_start;
	dest = addr_sdram;
	//memcpy(dest, source, 10240);
	int index = 0;
	dlog_info("start copy flash data\n");
	for (; index < 100 ;)
	{
		buff[index] = *source;
		source++;
		index++;
	}
	dlog_info("buff addr = %x\n", buff);
	/* test memcpy success*/
	dlog_info("the mem data is ");
	for (i = 0; i < 32; ++i)
	{
		dlog_info("data = %d\n", buff[i]);
	}

	sd_init();
 	sd_write(0x00000000, buff, 1);  /*byte units */
 	// sd_write(0x00001400, addr_sdram, 10); /* byte units*/
 	// sd_write(0x00002800, addr_sdram, 10); /* byte units*/

	sd_read(dest, 0x00000000, 1);
	for (i = 0; i < 36; ++i)
	{
		dlog_info("data = %d\n", dest[i]);
	}
	// sd_read(buff,  0x00001400, 5);  /* byte units*/
	// for (int i = 0; i < 36; ++i)
	// {
	// 	dlog_info("\ndata = ", dest[i]); 	
	// }

	sd_deinit();
}

void TestFatFs()
{
	// FRESULT res;                                          /* FatFs function common result code */
	// uint32_t byteswritten, bytesread;                     /* File write/read counts */
	// uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	// uint8_t rtext[100];                                   /* File read buffer */
	// /*##-1- Link the micro SD disk I/O driver ##################################*/
	// if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
	// {
	// 	serial_puts("\nLink success!\n");
	// 	/*##-2- Register the file system object to the FatFs module ##############*/
	// 	if ((res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
	// 	{
	// 		/* FatFs Initialization Error */
	// 		serial_puts("f_mount = ");
	// 		print_str(res);
	// 		serial_putc('\n');
	// 		serial_puts("f_mount error!\n");
	// 	}
	// 	else
	// 	{
	// 		serial_puts("\nf_mount success!\n");
	// 		res = f_mkfs((TCHAR const*)SDPath, 0, 0);
	// 		serial_puts("f_mkfs = ");
	// 		print_str(res);
	// 		/*##-4- Create and Open a new text file object with write access #####*/
	// 		if (f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
	// 		{
	// 			/* 'STM32.TXT' file Open for write Error */
	// 			serial_puts("\nf_open error!\n");
	// 		}

	// 		else
	// 		{
	// 			serial_puts("\nf_open success!\n");
	// 			/*##-5- Write data to the text file ################################*/
	// 			res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

	// 			if ((byteswritten == 0) || (res != FR_OK))
	// 			{
	// 				 'STM32.TXT' file Write or EOF Error 
	// 				serial_puts("\nf_write error!\n");
	// 			}
	// 			else
	// 			{
	// 				serial_puts("\nf_write success!\n");
	// 				/*##-6- Close the open text file #################################*/
	// 				f_close(&MyFile);

	// 				/*##-7- Open the text file object with read access ###############*/
	// 				if (f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
	// 				{
	// 					/* 'STM32.TXT' file Open for read Error */
	// 					serial_puts("\nf_open error!\n");
	// 				}
	// 				else
	// 				{
	// 					/*##-8- Read data from the text file ###########################*/
	// 					res = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);

	// 					if ((bytesread == 0) || (res != FR_OK))
	// 					{
	// 						/* 'STM32.TXT' file Read or EOF Error */
	// 						serial_puts("\nf_read error!\n");
	// 					}
	// 					else
	// 					{
	// 						serial_puts("\nf_read success!\n");
	// 						/*##-9- Close the open text file #############################*/
	// 						f_close(&MyFile);

	// 						/*##-10- Compare read data with the expected data ############*/
	// 						if ((bytesread != byteswritten))
	// 						{
	// 							/* Read data is different from the expected data */
	// 							serial_puts("\nf_close error!\n");
	// 						}
	// 						else
	// 						{
	// 							/* Success of the demo: no error occurrence */
	// 							serial_puts("\nf_close success!\n");
	// 						}
	// 					}
	// 				}
	// 			}
	// 		}
		
	// 	}
	// }

	// /*##-11- Unlink the micro SD disk I/O driver ###############################*/
	// res = FATFS_UnLinkDriver(SDPath);
}
